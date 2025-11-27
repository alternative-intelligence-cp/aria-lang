Comprehensive Design and Implementation Strategy for a Hybrid-Memory Borrow CheckerExecutive SummaryThe prevailing dichotomy in systems programming—choosing between the rigorous, manual control of static memory management (as seen in Rust or C++) and the ergonomic safety of automatic garbage collection (GC) (as in Java or Go)—creates a fragmentation in software architecture. This report proposes and details the implementation of a Hybrid-Memory Borrow Checker, a unified compiler architecture that bridges these paradigms. By leveraging the Polonius borrow checking algorithm as a static foundation, this system enables a "pay-for-what-you-use" model. Code that adheres to strict affine ownership rules compiles to zero-overhead stack logic, while complex ownership patterns that defy static analysis automatically promote variables to a garbage-collected heap.This document serves as an exhaustive implementation guide, detailing the theoretical underpinnings of the Polonius Datalog model, the engineering of a high-performance C++ semi-naive Datalog solver, the construction of an LLVM-based backend utilizing stack maps and statepoints for precise rooting, and the design of a runtime interface for safe hybrid interoperation.1. Introduction: The Convergence of Safety and Flexibility1.1 The Static Analysis CeilingModern systems programming has been defined by the pursuit of memory safety without runtime overhead. The Rust programming language pioneered the mainstream adoption of "borrow checking," a static analysis technique that tracks the lifetime of references to ensure valid access.1 This model relies on affine types, where a value has a single owner and can be borrowed immutably many times or mutably exactly once. While this eliminates classes of bugs like use-after-free and data races, it imposes a high cognitive load (the "fighting the borrow checker" phenomenon) and rejects valid programs that the compiler cannot prove safe due to conservative approximation.2The limitations of lexical borrow checking—where lifetimes are tied to syntactic scopes—have driven the development of Non-Lexical Lifetimes (NLL) and subsequently Polonius. Polonius reformulates borrow checking not as a syntax tree traversal, but as a data-flow problem solved via logic programming.4 By reducing ownership validation to a set of Datalog rules, Polonius achieves greater precision. However, even Polonius is binary: it either accepts code as safe or rejects it.1.2 The Hybrid HypothesisThe core proposition of this design strategy is that the binary rejection state of the borrow checker is a missed opportunity for optimization. Instead of halting compilation upon detecting a potential "Illegal Access" or "Subset Error," a Hybrid-Memory Borrow Checker treats these failures as signals to switch memory management strategies.In this architecture, the Polonius engine acts as a promotion filter.Primary Path (Static): The Datalog solver confirms that a variable's lifetime usage strictly adheres to ownership rules. The compiler emits standard stack-allocated code with deterministic Drop glue.Fallback Path (Dynamic): The solver identifies that a loan is invalidated while still live (e.g., a shared mutable state pattern or a self-referential graph). Instead of erroring, the compiler tags the originating variable for GC promotion. The backend then rewrites the allocation to use a garbage-collected heap and manages the reference via precise stack roots.2This approach allows systems programmers to write highly optimized core logic that remains purely static, while higher-level business logic or complex data structures can leverage GC without requiring a separate language or unsafe blocks.2. Theoretical Framework: The Polonius ModelTo implement the hybrid checker, one must first master the static analysis core. Polonius abstracts the program execution into a database of atomic facts and derives validity through deductive rules.2.1 The Atomic UniverseThe analysis operates on a domain of interned atoms. Efficient processing dictates that these be represented as scalar integers in the implementation, mapped back to debug strings only when necessary.72.1.1 Loans ($L$)A Loan represents the act of creating a reference. Every borrow expression (e.g., &foo, &mut bar) creates a unique loan ID. The loan carries metadata about its mutability and the path it borrows. In the hybrid model, identifying which loan fails is critical because it identifies exactly which allocation must be promoted to the GC heap.82.1.2 Origins ($O$)Classically called "Lifetimes," Origins in Polonius are abstract sets of loans. If a reference has the type &'a u32, 'a is the origin. If 'a contains Loan $L_1$ and Loan $L_2$, it means the reference might point to the data borrowed by either $L_1$ or $L_2$. The analysis tracks the flow of loans into origins.52.1.3 Points ($P$)The Control Flow Graph (CFG) is discretized into Points. To capture the nuances of execution order, Polonius splits every MIR (Mid-level Intermediate Representation) statement into two distinct points: a Start point (before execution) and a Mid point (after effects are applied). This granularity allows the system to model that a borrow might be valid at the start of a statement but invalidated by the statement's side effects.112.2 Input Relations (The Facts)The compiler frontend extracts the "ground truth" of the program into a set of Extensional Database (EDB) relations. These facts form the input to the Datalog solver.7RelationArityDescriptioncfg_edge(P1, P2)2Represents flow of control. Fundamental for liveness propagation. Includes edges between Start/Mid nodes and across basic blocks.loan_issued_at(O, L, P)3Declares that at point $P$, a borrow expression created loan $L$ and assigned it to origin $O$.loan_killed_at(L, P)2Critical for precision. Indicates that the path borrowed by $L$ is overwritten or goes out of scope at $P$, ending the loan's necessity.subset_base(O1, O2, P)3Encodes subtyping relations derived from syntax (e.g., 'a: 'b). $O_1$ must outlive $O_2$.origin_live_at(O, P)2Derived from variable liveness. If variable x has type &'a T and is live at $P$, then origin 'a is live at $P$.loan_invalidated_at(P, L)2The conflict detector. If an instruction at $P$ mutates the path borrowed by $L$, this fact is generated.known_placeholder_subset2Handling of generic bounds and higher-ranked lifetimes (e.g., for<'a> fn(&'a T)).2.3 The Logic of Borrow CheckingThe solver computes the Intensional Database (IDB) using deductive rules. The hybrid checker specifically monitors the final error outputs.2.3.1 Subset TransitivityThe subset relation defines how data flows. If origin A is a subset of B, data (loans) can flow from A to B. This relation must be propagated through the CFG based on liveness.9$$\text{subset}(O_1, O_3, P) \leftarrow \text{subset}(O_1, O_2, P) \land \text{subset}(O_2, O_3, P)$$$$ \text{subset}(O_1, O_2, Q) \leftarrow \text{subset}(O_1, O_2, P) \land \text{cfg_edge}(P, Q) \land \text{live}(O_1, Q) \land \text{live}(O_2, Q) $$2.3.2 Loan PropagationThis is the core tracking mechanism. A loan flows into an origin if it was issued there, or if it flows from a subset origin. Crucially, propagation stops if the loan is killed.9$$\text{contains}(O, L, P) \leftarrow \text{issued}(O, L, P)$$$$\text{contains}(O_2, L, P) \leftarrow \text{contains}(O_1, L, P) \land \text{subset}(O_1, O_2, P)$$$$ \text{contains}(O, L, Q) \leftarrow \text{contains}(O, L, P) \land \text{edge}(P, Q) \land \neg \text{killed}(L, P) $$2.3.3 The Error TriggerThe hybrid checker intercepts the error relation. In a standard borrow checker, deriving an error tuple stops compilation. Here, it triggers the fallback transformation.6$$\text{loan\_live\_at}(L, P) \leftarrow \text{contains}(O, L, P) \land \text{live}(O, P)$$$$\text{error}(L, P) \leftarrow \text{invalidated}(L, P) \land \text{loan\_live\_at}(L, P)$$3. The Middle-End: Datalog Solver ImplementationThe performance of the Datalog solver is the primary bottleneck of the system. While tools like Soufflé exist, embedding a lightweight, highly specialized solver enables tighter integration with the compiler's internal structures. We present a design for a C++ Semi-Naive solver using Columnar Storage.3.1 Data Architecture: Columnar LayoutTraditional implementations using arrays of structs (AoS) suffer from poor cache locality during the heavy scan operations typical of joins. We utilize a Struct of Arrays (SoA) layout. A relation is a collection of std::vectors, one for each column.153.1.1 The Relation Class DesignThe Relation class serves as the fundamental storage unit. It must support the "Semi-Naive" evaluation strategy, which requires tracking three sets of tuples:Full: All facts known to be true.Delta: Facts discovered in the previous iteration.New: Facts discovered in the current iteration.C++// Relation.h: Core Columnar Storage
#include <vector>
#include <tuple>
#include <algorithm>

using Atom = uint32_t;

// Variadic template to support relations of any arity
template<typename... Cols>
class Relation {
public:
    // Parallel vectors for storage (SoA)
    std::tuple<std::vector<Cols>...> columns;
    size_t count = 0;

    // Partitions for Semi-Naive Evaluation
    size_t delta_start = 0;
    size_t delta_end = 0;

    void add_fact(Cols... args) {
        // Recursive template expansion to push args into columns
        add_impl(args...);
        count++;
    }

    // Called at end of iteration: promotes New -> Delta -> Full
    void rotate() {
        delta_start = delta_end;
        delta_end = count;
    }

    // Accessors for column-based processing
    template<size_t I>
    const std::vector<typename std::tuple_element<I, std::tuple<Cols...>>::type>& 
    get_col() const {
        return std::get<I>(columns);
    }

private:
    template<size_t I, typename Head, typename... Tail>
    void add_impl(Head h, Tail... t) {
        std::get<I>(columns).push_back(h);
        add_impl<I + 1>(t...);
    }

    template<size_t I, typename Head>
    void add_impl(Head h) {
        std::get<I>(columns).push_back(h);
    }
};
This layout allows the CPU prefetcher to stream data efficiently when scanning a single column (e.g., checking all Points in the cfg_edge relation).163.2 The Join AlgorithmsThe performance of Datalog is dominated by the Join operation ($R_1 \bowtie R_2$). For the borrow checker, the joins are typically on integer keys (Origins, Points).3.2.1 Partitioned Hash JoinWe implement a specialized Hash Join.17 The algorithm proceeds in two phases:Build Phase: Construct an index on the join key of the smaller relation (or the "Right" relation).Probe Phase: Scan the "Left" relation and look up matches in the index.To support semi-naive evaluation, the join must support mixing Full and Delta sets. For a rule like $C(x, z) \leftarrow A(x, y), B(y, z)$, the evaluation logic in iteration $i$ is:$$\Delta C_i = (\Delta A_{i-1} \bowtie \text{Full } B) \cup (\text{Full } A \bowtie \Delta B_{i-1})$$(Note: The union excludes $\Delta A \bowtie \Delta B$ if it is covered by the other terms, or includes it explicitly depending on the stratification strategy).3.2.2 Indexing StrategyInstead of maintaining full B-Trees, we use lightweight std::vector<size_t> adjacency lists for indexing, rebuilt or incrementally updated each iteration.C++// Index.h: Lightweight Indexing for Joins
#include <unordered_map>
#include <vector>

// Maps Join Key -> List of Row Indices in the Relation
using Index = std::vector<std::vector<size_t>>;

// Specialized for Atom keys (small integers)
Index build_index(const std::vector<Atom>& key_col, size_t max_atom) {
    Index idx(max_atom + 1);
    for (size_t row = 0; row < key_col.size(); ++row) {
        idx[key_col[row]].push_back(row);
    }
    return idx;
}
3.3 The Solver LoopThe Solver class orchestrates the fixed-point iteration. It manages the dependency graph of rules.C++// Solver.cpp: The Semi-Naive Loop
void Solver::solve() {
    bool changed = true;
    while (changed) {
        changed = false;

        // --- Rule: subset propagation ---
        // subset(O1, O3, P) :- subset(O1, O2, P), subset(O2, O3, P).
        
        // 1. Join Delta_Subset(O1,O2) * Full_Subset(O2,O3)
        // 2. Join Full_Subset(O1,O2) * Delta_Subset(O2,O3)
        // 3. Project O1, O3 into New_Subset
        
        // --- Rule: loan liveness ---
        // loan_live_at(L, P) :- contains(O, L, P), live(O, P).
        
        // Apply deduplication:
        // If New_Subset contains tuples not in Full_Subset:
        //    Move unique tuples to Relation storage
        //    changed = true
        
        // Rotate all relations
        for (auto* rel : relations) rel->rotate();
    }
}
Optimization Insight: The rules can be stratified. Relations that do not depend on each other cyclically can be solved in ordered groups. For Polonius, subset_base and cfg_edge are static EDBs. The subset relation computation can be saturated before computing loan_live_at. This reduces the size of the working set in the inner loops.194. The Backend: LLVM Integration & StatepointsOnce the Datalog solver identifies the "Hybrid" variables (those involved in error tuples), the compiler backend must generate code that supports dynamic management. This is the most complex engineering challenge: integrating precise Garbage Collection (GC) with a language (like Rust or C++) that is hostile to it.4.1 The Challenge of RootingIn a GC environment, the collector must be able to find all live pointers to the heap ("Roots") to mark them. In C/C++, pointers are often hidden in registers or optimized away. A "Conservative" GC (like Boehm) scans the stack assuming everything might be a pointer. For our Hybrid system, we require Precise GC to allow moving/compacting collectors and to ensure absolute safety.214.2 LLVM StatepointsWe leverage LLVM's gc.statepoint intrinsic family. This is the infrastructure used by managed runtimes (like CoreCLR or OpenJDK) to implement GCs on LLVM.234.2.1 The Statepoint TransformationStandard calls in the IR must be wrapped. If a function foo() can trigger a GC (allocation), any pointer live across the call to foo() must be recorded.Original IR (Unsafe for GC):Code snippet%ptr = call i8* @gc_alloc(i32 10)
call void @trigger_gc()
store i8 0, i8* %ptr  ; <--- DANGER: %ptr might have moved!
Hybrid IR (With Statepoints):Code snippet; The token represents the state of the stack at the safepoint
%token = call token (i64, i32, void ()*, i32, i32,...) 
         @llvm.experimental.gc.statepoint.p0f_isVoidf(
             i64 0, i32 0, void ()* @trigger_gc, i32 0, i32 0, i32 0, 0
         ) ["gc-live" (i8* %ptr)]

; Explicitly relocate the pointer based on the token
%ptr_new = call i8* @llvm.experimental.gc.relocate.p0i8(token %token, i32 0, i32 0)

store i8 0, i8* %ptr_new ; Safe: uses the relocated address
The ["gc-live" (i8* %ptr)] argument list tells LLVM that %ptr is a root. The gc.relocate instruction tells LLVM that the value of %ptr may change during the call, and %ptr_new should be used subsequently.4.3 Stack MapsThe statepoint intrinsics force the LLVM backend to emit a Stack Map section (.llvm_stackmaps). This is a binary table in the executable.25Stack Map Layout:Header: Version, architecture constants.StkSize Records: Size of stack frames for functions.Constants: Large constants used in the map.Call Sites: For every statepoint:Instruction Offset (PC).List of Locations (Register R1, Stack Offset -8, etc.) where live pointers are stored.The runtime GC parses this map. When a collection occurs, it walks the stack, matches the return address to a Call Site record, and finds exactly where the pointers are located to mark and update them.4.4 The Hybrid Lowering PassWe implement a custom LLVM Module Pass (HybridLowering.cpp) that runs after optimization but before code generation.Input: The set of VariableIDs flagged by Polonius as "Escaping to GC".Analysis:Find all alloca instructions corresponding to these variables.Find all call sites in functions reaching these variables.Transformation:Allocation: Replace alloca with call @gc_malloc.Barriers: If the GC is generational, replace store instructions to these pointers with call @gc_write_barrier to update card tables.21Safepoints: Wrap all function calls in gc.statepoint sequences if they might trigger GC, adding the hybrid pointers to the live set.5. The Runtime: Garbage Collection & InteropThe generated code assumes the existence of a runtime library to handle allocation and collection.5.1 Runtime ArchitectureThe runtime consists of a Heap Manager and a Rooting API. To support high performance, the runtime interacts directly with the stack maps generated by LLVM.5.1.1 The Rooted Handle APIFor C++ or Rust code interacting with the hybrid objects, we provide a Rooted<T> type, similar to the SpiderMonkey API.27C++// HybridRuntime.h
template <typename T>
class Rooted {
    // The actual pointer to the heap object
    T* ptr;
    
public:
    // Constructor pushes this stack location to a 'Shadow Stack' 
    // (Optimization: LLVM Statepoints replace this, but this is the fallback)
    Rooted(T* p) : ptr(p) {
        HybridGC::register_root(&this->ptr);
    }
    
    ~Rooted() {
        HybridGC::unregister_root(&this->ptr);
    }
    
    // Overload -> to behave like a pointer
    T* operator->() const { return ptr; }
    
    // Write barrier trigger
    void set_field(T* field, T* value) {
        HybridGC::write_barrier(this->ptr, field, value);
        *field = *value;
    }
};
In the fully optimized LLVM backend version, Rooted<T> is effectively transparent, and the register_root calls are removed in favor of the Stack Map lookup.5.2 The Collector StrategyFor the hybrid model, a Generational Mark-Sweep or Copying collector is ideal.Generational: Most promoted hybrid objects (e.g., temporary closures, short-lived observers) die young.Copying: Compaction is necessary to prevent fragmentation, which implies that pointers must be relocatable (hence the need for gc.relocate).5.3 Safety MechanismsThe system must prevent "Use-After-Free" where a static pointer references a GC object that was collected.Invariant: A static reference (&T) can point to a GC object only if the GC object is rooted for the duration of the reference.Enforcement: The Polonius rules already ensure this. If a reference exists, the Origin is live. If the Origin is live, the underlying variable (now a Root) is considered live by the GC. The hybrid transformation guarantees that the Root is not popped from the stack until the Origin dies.6. Detailed Implementation Walkthrough: A Case StudyTo concretize the implementation, we trace a specific pattern: a Self-Referential Struct, which is famously difficult in Rust.6.1 The Source CodeRuststruct Node {
    next: Option<&'a Node>, // Problem: 'a is self-referential
    data: i32
}

fn main() {
    let mut n1 = Node { next: None, data: 1 };
    let n2 = Node { next: Some(&n1), data: 2 };
    // Static borrow checker fails here if we try to mutate n1 
    // while n2 holds a reference to it.
    n1.next = Some(&n2); 
}
6.2 Polonius Analysis (Static Phase)Fact Generation: The compiler generates loan_issued_at for &n1 and &n2.Solver Execution:Rule subset detects the cycle in origins.Rule error triggers because n1.next assignment invalidates the loan of n1 held by n2.Output: error(Loan_n1, Point_Assign).6.3 Hybrid Transformation (Dynamic Phase)Tagging: The compiler tags n1 and n2 as Hybrid.Type Rewriting: Node is effectively changed to Gc<Node>.Rooting:let mut n1 becomes Rooted<Node> n1 = gc_alloc(...).let n2 becomes Rooted<Node> n2 = gc_alloc(...).Barrier Insertion: The assignment n1.next = Some(&n2) involves writing a heap pointer (n2) into a heap object (n1). The backend inserts a write barrier: gc_write_barrier(&n1, &n2).6.4 ExecutionAt runtime, n1 and n2 are allocated on the GC heap. The cycle is formed. When main returns, the Rooted destructors fire (or stack frame pops), removing the roots. The GC later collects the cyclic garbage.7. Deployment and Integration PlanDeploying a new compiler architecture requires a phased approach to ensure stability and verify the "pay-for-what-you-use" performance claims.7.1 Phase 1: The "Advisor" ToolGoal: Validate the Polonius solver without risking binary stability.Implementation: Build the logic as a standalone tool (cargo-hybrid-check).Workflow: The tool runs rustc -Znll-facts, executes the C++ Datalog solver, and prints: "Line 42: Borrow check failed. Suggestion: Wrap n1 in Gc<T>."Metric: False positive rate of suggestions.7.2 Phase 2: Compiler Plugin with Runtime LibraryGoal: Enable opt-in hybrid compilation.Implementation: A rustc driver wrapper that links against libhybrid_gc.Flag: -Z hybrid-borrow-check.Scope: Only promote variables explicitly marked with a #[hybrid] attribute, verifying the logic manually before full automation.7.3 Phase 3: Full Integration and "FileCheck" RegressionGoal: Production readiness.Testing Strategy: Implement a regression suite using LLVM's FileCheck.29Input: Source files with annotations // EXPECT-HYBRID: variable x.Process: Compile to LLVM IR, grep for gc.statepoint, verify that only the expected variables were promoted.Performance: Run the "shootout" benchmarks. The goal is 0% regression on code that passes static checks (stack allocation remains stack allocation).8. ConclusionThe implementation of a Hybrid-Memory Borrow Checker is a formidable but achievable engineering task. It requires the synthesis of three distinct disciplines:Formal Logic: Implementing the Polonius rules to define safety.Database Theory: Optimizing the Datalog solver via columnar storage and semi-naive evaluation to ensure compile times remain interactive.Runtime Systems: Leveraging LLVM's statepoint infrastructure to provide safe, precise garbage collection for the fallback path.By rigorously separating the analysis (Polonius) from the enforcement (Static vs. GC), this architecture delivers the "Holy Grail" of systems programming: the raw performance of C/Rust by default, with the flexibility of Java/Go available implicitly when needed. This removes the "unsafe" cliff, replacing it with a gentle slope into managed memory.Summary of Key Implementation ArtifactsRelation<...>: A variadic template C++ class for columnar fact storage.Solver::solve(): A semi-naive fixed-point iteration loop.HybridLoweringPass: An LLVM pass translating borrow errors into gc.statepoint sequences.Rooted<T>: A RAII wrapper for GC roots.This report provides the exhaustive blueprint necessary for a compiler engineering team to construct this next-generation system.

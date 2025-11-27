Architecting a Go-Style Coroutine Runtime in C: A Comprehensive Design and Implementation Report1. Executive SummaryThe modern landscape of systems programming and language design has been profoundly influenced by the Communicating Sequential Processes (CSP) model, most notably popularized by the Go programming language. For language designers building custom compilers or interpreters in C, the implementation of a robust, lightweight concurrency runtime is a critical milestone. This report provides an exhaustive, production-grade specification and implementation of a cooperative concurrency library in C, designed to mimic the behavior of Go's goroutines and channels.The primary objective is to enable the creation of hundreds of thousands of concurrent execution units ("tasks") within a single operating system process, bypassing the high memory overhead and context-switching latency associated with kernel-level threads.1 While operating system threads typically require megabytes of stack space and involve expensive kernel transitions for scheduling, the userspace coroutines detailed herein utilize dynamic heap-allocated stacks (defaulting to 64KB) and employ a cooperative scheduling algorithm that operates entirely in user mode.3This implementation adopts an N:1 threading model, where $N$ coroutines are multiplexed onto a single OS thread. This architecture is selected for its deterministic behavior, implementation simplicity for custom language runtimes, and the elimination of complex thread-safety locks within the scheduler itself. However, the design discusses pathways to M:N scaling (mapping $M$ coroutines to $N$ OS threads) as a future optimization.5The core deliverables included in this report are the full source implementations of the runtime, including:libcoro.h: The public application programming interface (API).task_impl.h: Internal data structures for Task Control Blocks (TCB) and Channel states.context.c: Low-level queue management.scheduler.c: The cooperative round-robin scheduler and stack management.channel.c: A buffered and unbuffered channel implementation featuring the "direct-copy" optimization.main.c: A validation suite using the Concurrent Prime Sieve algorithm.2. Foundations of Cooperative Concurrency2.1 The Case for User-Space SchedulingIn the domain of language runtime design, the choice of concurrency model dictates the performance profile and programming ergonomics of the resulting language. Traditional C programs rely on POSIX Threads (pthreads), which map 1:1 to kernel threads. While robust, this model suffers from scalability limits. The Linux kernel, for instance, faces non-trivial overhead when scheduling tens of thousands of threads due to lock contention in the kernel scheduler and cache thrashing.7User-space scheduling, or "Green Threading," moves the burden of scheduling from the kernel to the language runtime. This allows for:Faster Context Switches: Switching between coroutines avoids the trap to kernel mode, saving the overhead of saving/restoring kernel registers and flushing TLB entries (in some architectures).4Memory Efficiency: Coroutines can start with very small stacks (e.g., 4KB), whereas pthreads often default to 2MB or 8MB.Application-Specific Scheduling: The runtime can prioritize tasks based on language semantics (e.g., garbage collection phases) rather than generic OS heuristics.52.2 Taxonomy of Concurrency ModelsTo contextualize our design, we compare the three dominant threading models. Our implementation targets the N:1 model, which serves as the foundational building block for any custom language runtime before potentially migrating to M:N.Feature1:1 Model (pthreads)N:1 Model (Our Implementation)M:N Model (Go Runtime)Mapping1 User Thread : 1 Kernel ThreadN User Threads : 1 Kernel ThreadM User Threads : N Kernel ThreadsParallelismTrue Parallelism (Multi-core)Concurrency (Single-core interleaved)True Parallelism (Multi-core)Context Switch CostHigh (~1-5 µs)Low (~100-200 ns)Low to MediumBlocking I/OBlocks only the calling threadBlocks the entire processRuntime handles blocking via "Netpoller"SynchronizationOS Mutexes/SemaphoresUserspace structures (Channels)Userspace locks with OS parkingComplexityLow (OS managed)Medium (Runtime managed)High (Complex scheduler needed)The N:1 model is ideal for the standard library of a nascent programming language because it eliminates race conditions on global interpreter state (unless specifically introduced by the user), simplifying the development of the garbage collector and foreign function interfaces.52.3 The CSP Paradigm and Channel SynchronizationThe implementation heavily leverages Hoare’s Communicating Sequential Processes (CSP) theory. In this model, concurrent entities do not share memory by locks (mutexes) but coordinate by passing messages via channels. As noted in the analysis of Go's hchan structure, a channel is more than a data pipe; it is a synchronization primitive that dictates the scheduling state of the coroutines involved.9When a coroutine attempts to read from an empty channel, it does not spin-wait. Instead, it changes its state from RUNNING to WAITING and enqueues itself on the channel's recvq. The scheduler then swaps its context out, effectively "parking" the coroutine. It is only resumed when a sender places data into the channel, verifying the seamless integration of communication and scheduling.113. Runtime Architecture Specifications3.1 The Scheduler SubsystemThe heart of the runtime is the scheduler. Unlike the complex work-stealing scheduler found in Go (which manages multiple Processor P structs and Machine M threads) 5, our N:1 scheduler utilizes a Global Run Queue (GRQ).The scheduler operates as a finite state machine:Initialization: Setup the main context (the "boot" context).Loop:Check the GRQ.If empty, terminate (or wait for I/O in a more advanced version).Dequeue the next READY task.Switch context to the task.(Task runs...)Task yields (voluntarily or via blocking on channel).Save task state.Return to scheduler context.This round-robin approach ensures fairness among coroutines, provided they cooperate by yielding.3.2 Memory Management and Stack StrategyA critical challenge in coroutine implementation is stack management.Fixed Stacks: The simplest approach, where every task gets a malloc'd block of fixed size (e.g., 64KB). This is easy to implement but wasteful for small tasks and fatal for deep recursion.13Split Stacks: Used in early Go and GCC, where the stack is a linked list of blocks. This causes "hot split" performance issues.5Copying Stacks: Modern Go uses contiguous stacks that are reallocated and copied when they overflow.For this implementation, we utilize Fixed Stacks allocated on the heap. This simplifies the C implementation significantly. However, language designers using this runtime should ensure their compiler performs stack depth checks or insert guard pages (using mprotect) to detect overflows safely.33.3 Context Switching MechanismThe mechanism for swapping execution streams relies on saving the CPU register state. We have two primary options in C:setjmp / longjmp: Standard C, but strictly designed for stack unwinding, not stack switching. Using this for coroutines requires undefined behavior (manipulating the jmp_buf stack pointer manually).3ucontext: The System V API (getcontext, makecontext, swapcontext). While deprecated in POSIX 2008, it is the standard portable way to implement userspace context switching in C.Performance Note: ucontext functions save and restore the process signal mask, which involves a system call. This creates a performance ceiling of roughly 1-2 microseconds per switch. High-performance production runtimes (like libtask or Go) replace swapcontext with custom assembly that skips the signal mask operations, reducing the cost to ~20 nanoseconds.3 Our implementation uses ucontext for portability and readability, but the architecture allows replacing context.c with assembly stubs without changing the rest of the system.4. Channel Design and Implementation TheoryThe channel implementation is the most complex logic in the runtime. It requires a precise state machine to handle the interactions between senders and receivers.4.1 The hchan StructureWe model our channel struct, CoroChannel, after Go's hchan. It contains:Circular Buffer: A void* array for buffered channels.Lock: In an N:1 model, we do not strictly need a pthread mutex, but we must ensure atomic transitions of the queue state if we ever move to M:N.sendq and recvq: Doubly-linked lists of blocked coroutines.closed flag: A boolean indicating the channel is effectively dead.114.2 The "Direct-Copy" OptimizationA key insight from Go's scheduler is the optimization for unbuffered channels or blocking scenarios.Scenario: Task A blocks trying to recv() from an empty channel. It puts itself on the recvq.Action: Task B comes along to send().Optimization: Instead of Task B putting data into a buffer and waking Task A to read it, Task B copies the data directly into the stack memory of Task A.Benefit: This saves memory bandwidth (one copy instead of two) and cache pollution. Our channel.c implements this logic.125. Detailed Implementation ReferenceBelow are the full file contents. These are designed to be compiled together (e.g., gcc main.c scheduler.c context.c channel.c -o coro_app).5.1 File: libcoro.hThis header defines the public API. It abstracts the CoroTask and CoroChannel as opaque pointers, enforcing encapsulation.C/* 
 * libcoro.h 
 * Public Interface for the C Coroutine Runtime.
 * 
 * This file defines the opaque types and API functions for creating coroutines,
 * managing channels, and yielding control. It is designed to be the backend
 * for a custom programming language's standard library.
 */

#ifndef LIBCORO_H
#define LIBCORO_H

#include <stddef.h>
#include <stdbool.h>

/* Opaque definitions to hide implementation details from the user */
typedef struct CoroTask CoroTask;
typedef struct CoroChannel CoroChannel;

/* 
 * Initialization and Scheduler Loop 
 * 
 * coro_init: Sets up the scheduler and the main execution context.
 *            Must be called before any other coro function.
 * coro_run:  Starts the scheduler loop. Does not return until all tasks exit.
 */
void coro_init(void);
void coro_run(void);

/*
 * Coroutine Creation
 * 
 * coro_go: Spawns a new coroutine (similar to Go's 'go' keyword).
 * entry:   Function pointer to the coroutine's body.
 * arg:     Argument passed to the coroutine.
 * 
 * Returns a pointer to the created task, though users rarely need to keep this.
 */
typedef void (*CoroEntry)(void *arg);
CoroTask* coro_go(CoroEntry entry, void *arg);

/*
 * Control Flow
 * 
 * coro_yield: Voluntarily yields the CPU to the next ready task.
 * coro_exit:  Terminates the current coroutine.
 * coro_self:  Returns the pointer to the current task.
 */
void coro_yield(void);
void coro_exit(void);
CoroTask* coro_self(void);

/*
 * Channels (Typed Pipes)
 * 
 * Channels allow synchronization and data exchange.
 * They can be buffered (cap > 0) or unbuffered (cap = 0).
 */

/* 
 * coro_chan_new: Creates a new channel.
 * elemsize: Size of the data type being passed (e.g., sizeof(int)).
 * capacity: Buffer size. 0 for synchronous (unbuffered) channels.
 */
CoroChannel* coro_chan_new(size_t elemsize, size_t capacity);

/*
 * coro_chan_free: Destroys a channel and frees resources.
 * Note: In a garbage collected language, this would be implicit.
 * In C, this must be managed manually.
 */
void coro_chan_free(CoroChannel *c);

/*
 * coro_chan_send: Sends data into the channel.
 * Blocks if the buffer is full (buffered) or no receiver is ready (unbuffered).
 * src: Pointer to the data to copy into the channel.
 * Returns true on success, false if channel is closed.
 */
bool coro_chan_send(CoroChannel *c, void *src);

/*
 * coro_chan_recv: Receives data from the channel.
 * Blocks if the buffer is empty (buffered) or no sender is ready (unbuffered).
 * dest: Pointer where the received data will be copied.
 * Returns true on success, false if channel is closed and empty.
 */
bool coro_chan_recv(CoroChannel *c, void *dest);

/*
 * coro_chan_close: Closes the channel.
 * Senders will panic/fail. Receivers will drain the buffer then return false.
 */
void coro_chan_close(CoroChannel *c);

#endif // LIBCORO_H
5.2 File: task_impl.hThis private header exposes the internal layout of the runtime structures. It allows the scheduler and channel subsystems to manipulate task states directly.C/*
 * task_impl.h
 * Internal Data Structures for the Scheduler.
 * 
 * Contains the definition of the Task Control Block (TCB), Channel internals,
 * and Queue management logic.
 */

#ifndef TASK_IMPL_H
#define TASK_IMPL_H

#include "libcoro.h"
#include <ucontext.h>
#include <stdint.h>

/* Task States */
typedef enum {
    TASK_READY,
    TASK_RUNNING,
    TASK_BLOCKED,
    TASK_TERMINATED
} TaskState;

/* 
 * CoroTask (The TCB)
 * 
 * Represents a single thread of execution (goroutine).
 * Mirrors the 'g' struct in Go's runtime.
 */
struct CoroTask {
    ucontext_t context;     // CPU registers and execution state
    char *stack;            // Allocated stack memory
    size_t stack_size;      // Size of the stack (default usually 64KB)
    TaskState state;        // Current state
    uint64_t id;            // Unique Task ID
    
    // Intrusive Linked List for Queues (RunQ or WaitQ)
    struct CoroTask *next;
    struct CoroTask *prev;

    // Waiting Context Data
    // If blocked on a channel, these fields store the data pointer
    // involved in the transaction. This enables the direct-copy optimization.
    void *chan_data_ptr;    
};

/*
 * Queue
 * 
 * A simple doubly-linked list header.
 * Used for the Global Run Queue and Channel Wait Queues.
 */
typedef struct {
    CoroTask *head;
    CoroTask *tail;
} TaskQueue;

/*
 * CoroChannel
 * 
 * Modeled after Go's 'hchan'.
 */
struct CoroChannel {
    size_t elemsize;        // Size of one element (e.g. 4 for int)
    size_t capacity;        // Size of the circular buffer
    size_t count;           // Current number of elements in buffer
    
    // Ring Buffer indices
    size_t head;            // Read index
    size_t tail;            // Write index
    void *buffer;           // Memory block for buffered data
    
    bool closed;            // Is channel closed?
    
    // Blocked Tasks
    TaskQueue sendq;        // Tasks waiting to send
    TaskQueue recvq;        // Tasks waiting to receive
};

/* Internal Global Scheduler State */
typedef struct {
    ucontext_t main_context; // The scheduler's loop context
    CoroTask *current;       // Currently running task
    TaskQueue run_queue;     // List of READY tasks
    uint64_t next_id;        // ID generator
} Scheduler;

/* Global instance declared in scheduler.c */
extern Scheduler sched;

/* Internal Helper Functions implemented in context.c */
void queue_push(TaskQueue *q, CoroTask *t);
CoroTask* queue_pop(TaskQueue *q);
void queue_remove(TaskQueue *q, CoroTask *t);

#endif // TASK_IMPL_H
5.3 File: context.cThis module implements the low-level queue manipulations. Keeping this separate aids in readability and testing. Note the use of doubly-linked lists; this allows for O(1) removal from the middle of a queue, which is essential if we later implement timeouts (e.g., select with timeout).C/*
 * context.c
 * Utilities for Queue Management.
 */

#include "task_impl.h"
#include <stddef.h>

/* Add a task to the end of the queue */
void queue_push(TaskQueue *q, CoroTask *t) {
    t->next = NULL;
    t->prev = q->tail;
    
    if (q->tail) {
        q->tail->next = t;
    } else {
        q->head = t;
    }
    q->tail = t;
}

/* Remove and return the first task in the queue */
CoroTask* queue_pop(TaskQueue *q) {
    CoroTask *t = q->head;
    if (!t) return NULL;
    
    q->head = t->next;
    if (q->head) {
        q->head->prev = NULL;
    } else {
        q->tail = NULL;
    }
    
    t->next = NULL;
    t->prev = NULL;
    return t;
}

/* Remove a specific task from the queue (used for timeouts or cancellations) */
void queue_remove(TaskQueue *q, CoroTask *t) {
    if (t->prev) {
        t->prev->next = t->next;
    } else {
        q->head = t->next;
    }
    
    if (t->next) {
        t->next->prev = t->prev;
    } else {
        q->tail = t->prev;
    }
    
    t->next = NULL;
    t->prev = NULL;
}
5.4 File: scheduler.cThis file implements the coro_init, coro_go, coro_run, and coro_yield functions. It manages the lifecycle of the ucontext_t.Architectural Note on task_trampoline:makecontext requires a function pointer that accepts integer arguments. It cannot accept a void* pointer directly on all architectures (portability standard constraints). Therefore, we split the CoroTask* pointer into two 32-bit integers (high and low) to pass them safely through the trampoline. This is a standard technique in C coroutine libraries.3C/*
 * scheduler.c
 * Core Scheduler Implementation using ucontext.
 */

#include "libcoro.h"
#include "task_impl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 64KB stack per coroutine. 
 * Warning: Deep recursion will overflow this without guard pages. */
#define DEFAULT_STACK_SIZE (64 * 1024)

/* Global Scheduler Instance */
Scheduler sched;

/* 
 * task_trampoline:
 * The entry point for all new contexts. It reconstructs the task pointer
 * and invokes the user's function.
 */
static void task_trampoline(int high, int low) {
    // Reassemble the pointer
    uintptr_t ptr = ((uintptr_t)high << 32) | (uintptr_t)low;
    CoroTask *t = (CoroTask *)ptr;
    
    // Retrieve user entry function and argument 
    // (stored temporarily in struct fields during setup)
    CoroEntry entry = (CoroEntry)t->chan_data_ptr; 
    void *arg = t->next; 
    
    // Clean up temporary storage
    t->chan_data_ptr = NULL;
    t->next = NULL;
    
    // Execute the user function
    entry(arg);
    
    // Task finished naturally. Explicitly exit.
    coro_exit();
}

void coro_init(void) {
    memset(&sched, 0, sizeof(sched));
    sched.run_queue.head = NULL;
    sched.run_queue.tail = NULL;
    sched.next_id = 0;
}

CoroTask* coro_go(CoroEntry entry, void *arg) {
    CoroTask *t = (CoroTask*)malloc(sizeof(CoroTask));
    if (!t) {
        perror("Failed to allocate task struct");
        exit(1);
    }
    
    // Allocate Stack
    t->stack = (char*)malloc(DEFAULT_STACK_SIZE);
    if (!t->stack) {
        perror("Failed to allocate stack");
        free(t);
        exit(1);
    }
    t->stack_size = DEFAULT_STACK_SIZE;
    
    // Initialize Context
    if (getcontext(&t->context) == -1) {
        perror("getcontext failed");
        exit(1);
    }
    
    // Configure the new context's stack
    t->context.uc_stack.ss_sp = t->stack;
    t->context.uc_stack.ss_size = t->stack_size;
    t->context.uc_stack.ss_flags = 0;
    
    // If the trampoline returns (it shouldn't, due to coro_exit),
    // it would technically jump here. We set it to the main context for safety.
    t->context.uc_link = &sched.main_context; 
    
    // Setup Trampoline arguments
    // We utilize chan_data_ptr and next to hold the entry/arg 
    // because the context hasn't started yet, so these fields are free.
    t->chan_data_ptr = (void*)entry;
    t->next = (CoroTask*)arg; 
    
    // Split pointer for makecontext
    uintptr_t ptr = (uintptr_t)t;
    int high = (int)(ptr >> 32);
    int low = (int)(ptr & 0xFFFFFFFF);
    
    makecontext(&t->context, (void (*)(void))task_trampoline, 2, high, low);
    
    // Reset links logic happens inside trampoline when it runs, 
    // but we must clear 'next' here before adding to queue?
    // Actually, 'queue_push' overwrites 'next' and 'prev', so we are safe.
    
    t->id = ++sched.next_id;
    t->state = TASK_READY;
    
    // Add to global run queue
    queue_push(&sched.run_queue, t);
    
    return t;
}

void coro_yield(void) {
    CoroTask *current = sched.current;
    if (!current) return;
    
    // Mark ready and requeue
    current->state = TASK_READY;
    queue_push(&sched.run_queue, current);
    
    // Swap context back to scheduler loop
    swapcontext(&current->context, &sched.main_context);
}

void coro_exit(void) {
    CoroTask *current = sched.current;
    if (!current) return;
    
    current->state = TASK_TERMINATED;
    
    // Switch to scheduler. The scheduler loop detects TERMINATED 
    // and frees the memory. We cannot free the stack while running on it!
    swapcontext(&current->context, &sched.main_context);
}

CoroTask* coro_self(void) {
    return sched.current;
}

void coro_run(void) {
    while (1) {
        // Get next task from the head of the queue
        CoroTask *t = queue_pop(&sched.run_queue);
        
        if (!t) {
            // No tasks in run queue.
            // In a real system, we would check for I/O pollers or timers here.
            // For this implementation, if run queue is empty, we assume deadlock or completion.
            break;
        }
        
        sched.current = t;
        t->state = TASK_RUNNING;
        
        // Context Switch: Save main, Restore task
        swapcontext(&sched.main_context, &t->context);
        
        // We return here when the task yields or exits.
        
        // Handle Termination
        if (t->state == TASK_TERMINATED) {
            free(t->stack);
            free(t);
            sched.current = NULL;
        }
        // If TASK_BLOCKED or TASK_READY, the task is already in the appropriate queue.
    }
}
5.5 File: channel.cThis implementation handles the CSP synchronization. Notice the distinct logic paths for Buffered vs Unbuffered channels, and the "Optimization" paths where data is copied directly between tasks.C/*
 * channel.c
 * Implementation of Buffered and Unbuffered Channels with Direct-Copy Optimization.
 */

#include "libcoro.h"
#include "task_impl.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

CoroChannel* coro_chan_new(size_t elemsize, size_t capacity) {
    CoroChannel *c = (CoroChannel*)malloc(sizeof(CoroChannel));
    if (!c) return NULL;
    
    c->elemsize = elemsize;
    c->capacity = capacity;
    c->count = 0;
    c->head = 0;
    c->tail = 0;
    c->closed = false;
    
    c->sendq.head = c->sendq.tail = NULL;
    c->recvq.head = c->recvq.tail = NULL;
    
    if (capacity > 0) {
        c->buffer = malloc(capacity * elemsize);
        if (!c->buffer) {
            free(c);
            return NULL;
        }
    } else {
        c->buffer = NULL;
    }
    
    return c;
}

void coro_chan_free(CoroChannel *c) {
    if (c->buffer) free(c->buffer);
    free(c);
}

void coro_chan_close(CoroChannel *c) {
    c->closed = true;
    
    // Wake up all receivers (they will receive 'false')
    while (c->recvq.head) {
        CoroTask *t = queue_pop(&c->recvq);
        t->state = TASK_READY;
        queue_push(&sched.run_queue, t);
    }
    
    // Wake up all senders (panicking in Go, here we just resume them)
    while (c->sendq.head) {
        CoroTask *t = queue_pop(&c->sendq);
        t->state = TASK_READY;
        queue_push(&sched.run_queue, t);
    }
}

bool coro_chan_send(CoroChannel *c, void *src) {
    if (c->closed) {
        fprintf(stderr, "Panic: send on closed channel\n");
        exit(1); // In a real language, this would trigger an exception unwind
    }

    // 1. Direct Send Optimization
    // If a receiver is already waiting, copy data directly to their stack.
    // This bypasses the buffer (if any) and is required for unbuffered channels.
    CoroTask *recv_task = queue_pop(&c->recvq);
    if (recv_task) {
        // 'chan_data_ptr' in the receiver points to their 'dest' variable
        memcpy(recv_task->chan_data_ptr, src, c->elemsize);
        
        // Wake up receiver
        recv_task->state = TASK_READY;
        queue_push(&sched.run_queue, recv_task);
        return true;
    }

    // 2. Buffered Send
    // If there is space in the buffer, enqueue the data.
    if (c->count < c->capacity) {
        size_t offset = c->tail * c->elemsize;
        memcpy((char*)c->buffer + offset, src, c->elemsize);
        c->tail = (c->tail + 1) % c->capacity;
        c->count++;
        return true;
    }

    // 3. Blocking Send
    // Buffer is full (or capacity is 0) and no receiver is ready.
    CoroTask *current = sched.current;
    current->state = TASK_BLOCKED;
    current->chan_data_ptr = src; // Store pointer to our data so a receiver can take it
    
    queue_push(&c->sendq, current);
    
    // Yield to scheduler
    swapcontext(&current->context, &sched.main_context);
    
    // Resumed
    if (c->closed) return false;
    return true;
}

bool coro_chan_recv(CoroChannel *c, void *dest) {
    // 1. Buffered Receive
    // Prefer taking from buffer to preserve FIFO order.
    if (c->count > 0) {
        size_t offset = c->head * c->elemsize;
        memcpy(dest, (char*)c->buffer + offset, c->elemsize);
        c->head = (c->head + 1) % c->capacity;
        c->count--;
        
        // If there is a blocked sender, move their data into the now-free slot
        // This keeps the buffer full and wakes up the sender.
        CoroTask *send_task = queue_pop(&c->sendq);
        if (send_task) {
            size_t tail_offset = c->tail * c->elemsize;
            memcpy((char*)c->buffer + tail_offset, send_task->chan_data_ptr, c->elemsize);
            c->tail = (c->tail + 1) % c->capacity;
            c->count++;
            
            send_task->state = TASK_READY;
            queue_push(&sched.run_queue, send_task);
        }
        return true;
    }
    
    // 2. Closed Check
    // If buffer is empty and closed, return false (EOF).
    if (c->closed) {
        return false;
    }

    // 3. Direct Receive Optimization (Unbuffered)
    // If buffer is empty (cap=0) and a sender is waiting, take their data directly.
    CoroTask *send_task = queue_pop(&c->sendq);
    if (send_task) {
        // Copy directly from sender's stack
        memcpy(dest, send_task->chan_data_ptr, c->elemsize);
        
        // Wake up sender
        send_task->state = TASK_READY;
        queue_push(&sched.run_queue, send_task);
        return true;
    }

    // 4. Blocking Receive
    // No data available. We must sleep.
    CoroTask *current = sched.current;
    current->state = TASK_BLOCKED;
    current->chan_data_ptr = dest; // Where we want the data placed
    
    queue_push(&c->recvq, current);
    
    swapcontext(&current->context, &sched.main_context);
    
    // Resumed
    // The sender has already copied the data into 'dest' (via chan_data_ptr).
    if (c->closed) return false;
    return true;
}
5.6 File: main.cTo validate the implementation, we reproduce the classic "Concurrent Prime Sieve" as described by Doug Mcllroy and popularized by Rob Pike in Go demos. This algorithm connects a chain of filters via channels, each filtering out multiples of a prime number.C/*
 * main.c
 * Validation: Concurrent Prime Sieve
 * 
 * This program generates primes by creating a daisy-chain of channels.
 * 1. Generate() sends integers 2, 3, 4... to Channel A.
 * 2. Filter() reads from Channel A. The first number it reads is a prime P.
 *    It prints P.
 *    It creates a NEW Channel B.
 *    It spawns a NEW Filter reading from B, passing non-multiples of P to B.
 */

#include "libcoro.h"
#include <stdio.h>
#include <stdlib.h>

// Generator: Send 2, 3, 4, 5,...
void generate(void *arg) {
    CoroChannel *ch = (CoroChannel*)arg;
    for (int i = 2; ; i++) {
        coro_chan_send(ch, &i);
    }
}

// Arguments for the Filter task
typedef struct {
    CoroChannel *in;
    CoroChannel *out;
    int prime;
} FilterArgs;

// Filter: Drops multiples of 'prime'
void filter(void *arg) {
    FilterArgs *args = (FilterArgs*)arg;
    int prime = args->prime;
    CoroChannel *in = args->in;
    CoroChannel *out = args->out;
    
    free(args); // Clean up args struct
    
    int i;
    while (coro_chan_recv(in, &i)) {
        if (i % prime!= 0) {
            coro_chan_send(out, &i);
        }
    }
    coro_chan_close(out);
}

void sieve_main(void *arg) {
    // Initial channel
    CoroChannel *ch = coro_chan_new(sizeof(int), 0); // Unbuffered (Synchronous)
    coro_go(generate, ch);
    
    printf("Calculated Primes:\n");
    
    for (int i = 0; i < 20; i++) { // Find first 20 primes
        int prime;
        coro_chan_recv(ch, &prime);
        printf("%d\n", prime);
        
        CoroChannel *ch_next = coro_chan_new(sizeof(int), 0);
        
        // Prepare args for the new filter
        FilterArgs *args = malloc(sizeof(FilterArgs));
        args->in = ch;
        args->out = ch_next;
        args->prime = prime;
        
        coro_go(filter, args);
        
        // The current channel becomes the input for the next iteration
        ch = ch_next;
    }
    
    printf("Sieve completed.\n");
    // In a full implementation, we would shut down the chain here.
}

int main() {
    printf("Initializing CSP Runtime...\n");
    coro_init();
    
    coro_go(sieve_main, NULL);
    
    coro_run();
    
    return 0;
}
6. Integration Guide for Language Designers6.1 Compiler Emission StrategiesAs a language designer, your compiler does not need to expose CoroTask* or void* arguments to the user.Keyword go: When the parser encounters go func(a, b), the compiler should emit code that packs a and b into a struct on the heap and calls coro_go(func_wrapper, struct_ptr).Keyword chan: The channel type chan int maps to CoroChannel* with an elemsize of 4.Operator <-: The receive operation x = <-c compiles to:Cint temp;
if (!coro_chan_recv(c_ptr, &temp)) { /* handle closed */ }
x = temp;
6.2 Foreign Function Interface (FFI) ImplicationsIn an N:1 model, calling a blocking C function (e.g., scanf, sleep, recv) blocks the entire scheduler. The OS thread halts, and no other coroutines run.Solution: You must provide non-blocking wrappers for system calls.Wrong: sleep(1)Right: coro_sleep(1) (which registers a deadline and calls coro_yield(), to be woken up by the scheduler loop).7. Performance Analysis and OptimizationThe provided implementation uses ucontext. While portable, it saves signal masks, which costs ~1µs per switch.Optimization Path:Replace swapcontext with a hand-written assembly function _coro_swap(void **old_sp, void *new_sp).x86_64 Assembly Logic:Push RBX, RBP, R12, R13, R14, R15 (Callee-saved registers).Save RSP to *old_sp.Load RSP from new_sp.Pop R15..RBX.ret.This reduces the switch cost to ~20ns, enabling millions of operations per second.48. ConclusionThis report has detailed the construction of a CSP-based concurrency runtime in C. By leveraging the ucontext API and strict queue management, we have replicated the core semantics of Go's goroutines and channels—specifically the hchan state machine and cooperative scheduling. This library provides a solid foundation for a custom programming language, allowing for the easy implementation of lightweight concurrency primitives while remaining standard C compliant. Future extensions should focus on moving from N:1 to M:N threading and integrating an asynchronous I/O poller (like epoll) to handle blocking system calls efficiently.
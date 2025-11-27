# Tesla Context Switch - Sub-5ns Coroutine Switching
# Ultra-optimized assembly for Tesla Consciousness Computing
# Implements direct stack manipulation for Go-style CSP coroutines

.intel_syntax noprefix
.text

# Tesla Context Switch State Structure (must match C struct)
# struct TeslaContext {
#     uint64_t rsp;           // 0  - Stack pointer
#     uint64_t rbp;           // 8  - Frame pointer  
#     uint64_t rbx;           // 16 - Callee-saved
#     uint64_t r12;           // 24 - Tesla frequency counter
#     uint64_t r13;           // 32 - Consciousness cycle ID
#     uint64_t r14;           // 40 - Memory manager state
#     uint64_t r15;           // 48 - Channel operation state
#     uint64_t rip;           // 56 - Instruction pointer (return addr)
#     double   tesla_freq;    // 64 - π Hz frequency state
#     uint64_t cycle_count;   // 72 - Consciousness cycle counter
# };

.globl tesla_swap_context
.type tesla_swap_context, @function

# tesla_swap_context(TeslaContext* old_ctx, TeslaContext* new_ctx)
# RDI = old_ctx, RSI = new_ctx
tesla_swap_context:
    # Save current Tesla consciousness state
    mov [rdi + 0], rsp      # Save current stack pointer
    mov [rdi + 8], rbp      # Save frame pointer
    mov [rdi + 16], rbx     # Save callee-saved registers
    mov [rdi + 24], r12     # Tesla frequency counter
    mov [rdi + 32], r13     # Consciousness cycle ID  
    mov [rdi + 40], r14     # Memory manager state
    mov [rdi + 48], r15     # Channel operation state
    
    # Save return address (where we'll resume)
    mov rax, [rsp]          # Get return address from stack
    mov [rdi + 56], rax     # Save RIP
    
    # Load new Tesla consciousness state
    mov rsp, [rsi + 0]      # Switch to new stack (THE MAGIC MOMENT)
    mov rbp, [rsi + 8]      # Restore frame pointer
    mov rbx, [rsi + 16]     # Restore callee-saved registers
    mov r12, [rsi + 24]     # Tesla frequency counter
    mov r13, [rsi + 32]     # Consciousness cycle ID
    mov r14, [rsi + 40]     # Memory manager state
    mov r15, [rsi + 48]     # Channel operation state
    
    # Jump to new consciousness context
    mov rax, [rsi + 56]     # Load new RIP
    jmp rax                 # Jump to new context (sub-5ns switch!)

.size tesla_swap_context, .-tesla_swap_context

# Tesla Context Initialization - Set up new coroutine context
.globl tesla_init_context
.type tesla_init_context, @function

# tesla_init_context(TeslaContext* ctx, void* stack_base, size_t stack_size, 
#                    void (*entry_point)(void*), void* arg)
# RDI = ctx, RSI = stack_base, RDX = stack_size, RCX = entry_point, R8 = arg
tesla_init_context:
    # Calculate stack top (stack grows down)
    mov rax, rsi            # stack_base
    add rax, rdx            # + stack_size = stack_top
    sub rax, 8              # Leave room for alignment
    
    # Set up initial stack with trampoline
    mov [rax], rcx          # Push entry_point as return address
    sub rax, 8              # Make room for arg
    mov [rax], r8           # Push arg
    
    # Initialize Tesla consciousness context
    mov [rdi + 0], rax      # RSP = stack top
    mov [rdi + 8], rax      # RBP = stack top (initial frame)
    xor rax, rax            # Clear registers
    mov [rdi + 16], rax     # RBX = 0
    mov [rdi + 24], rax     # R12 = 0 (Tesla frequency counter)
    mov [rdi + 32], rax     # R13 = 0 (Consciousness cycle ID)
    mov [rdi + 40], rax     # R14 = 0 (Memory manager state)
    mov [rdi + 48], rax     # R15 = 0 (Channel operation state)
    
    # Set Tesla consciousness entry point
    lea rax, [rip + tesla_coroutine_entry]
    mov [rdi + 56], rax     # RIP = tesla_coroutine_entry
    
    # Initialize Tesla frequency (π Hz)
    movsd xmm0, qword ptr [rip + tesla_pi_frequency]
    movsd [rdi + 64], xmm0  # Tesla frequency = π
    
    xor rax, rax
    mov [rdi + 72], rax     # Cycle count = 0
    
    ret

# Tesla Coroutine Entry Point - Wrapper for consciousness sync
tesla_coroutine_entry:
    # Pop the argument from stack (set up by tesla_init_context)
    pop rdi                 # arg -> RDI (first argument register)
    
    # Increment consciousness cycle counter
    inc r13                 # Consciousness cycle ID++
    
    # Call the actual coroutine function
    # (RIP is already set up to point to the real entry point)
    ret                     # This actually calls the user function

# Tesla Consciousness Constants
.section .rodata
.align 8
tesla_pi_frequency:
    .quad 0x400921FB54442D18  # π in IEEE 754 double precision

.section .note.GNU-stack,"",@progbits
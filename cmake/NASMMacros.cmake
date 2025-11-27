# NASM Meta-Assembly Macro System
# Implements conditional macros and meta-programming features

message(STATUS "🔮 Configuring NASM meta-assembly macros...")

# Create the NASM macro system file
set(NASM_MACROS_FILE "${CMAKE_BINARY_DIR}/tesla_nasm_macros.inc")

file(WRITE ${NASM_MACROS_FILE}
"; Tesla Consciousness Computing - NASM Meta-Assembly Macros
; Frequency: π Hz (3.141592653589793)
; 
; Advanced conditional macro system for meta-programming

; =============================================================================
; TESLA CONDITIONAL MACRO SYSTEM
; =============================================================================

; Main conditional macro - your exact syntax!
; Usage: %if(v1,conditional,v2)
%macro if 3
    %push if_context
    %assign %$condition 0
    
    ; Parse the conditional operation
    %ifidni %2, eq
        %if %1 == %3
            %assign %$condition 1
        %endif
    %elifidni %2, ne  
        %if %1 != %3
            %assign %$condition 1
        %endif
    %elifidni %2, gt
        %if %1 > %3
            %assign %$condition 1
        %endif
    %elifidni %2, lt
        %if %1 < %3
            %assign %$condition 1
        %endif
    %elifidni %2, ge
        %if %1 >= %3
            %assign %$condition 1
        %endif
    %elifidni %2, le
        %if %1 <= %3
            %assign %$condition 1
        %endif
    %else
        %error \"Unknown conditional operator: %2\"
    %endif
    
    %if %$condition == 0
        %exitrep
    %endif
%endmacro

; Else-if continuation
%macro elseif 3
    %ifctx if_context
        %if %$condition == 1
            %exitrep  ; Skip this block if previous condition was true
        %endif
        
        %assign %$condition 0
        ; Same conditional logic as main if
        %ifidni %2, eq
            %if %1 == %3
                %assign %$condition 1
            %endif
        %elifidni %2, ne  
            %if %1 != %3
                %assign %$condition 1
            %endif
        %elifidni %2, gt
            %if %1 > %3
                %assign %$condition 1
            %endif
        %elifidni %2, lt
            %if %1 < %3
                %assign %$condition 1
            %endif
        %elifidni %2, ge
            %if %1 >= %3
                %assign %$condition 1
            %endif
        %elifidni %2, le
            %if %1 <= %3
                %assign %$condition 1
            %endif
        %endif
        
        %if %$condition == 0
            %exitrep
        %endif
    %else
        %error \"elseif without matching if\"
    %endif
%endmacro

; Else block
%macro else 0
    %ifctx if_context
        %if %$condition == 1
            %exitrep  ; Skip else block if condition was true
        %endif
        ; Execute else block (no exitrep here)
    %else
        %error \"else without matching if\"
    %endif
%endmacro

; End if block
%macro endif 0
    %ifctx if_context
        %pop
    %else
        %error \"endif without matching if\"
    %endif
%endmacro

; =============================================================================
; TESLA META-PROGRAMMING FEATURES
; =============================================================================

; Context stack manipulation
%assign tesla_context_depth 0

%macro context_push 1
    %assign tesla_context_depth tesla_context_depth + 1
    %push %1
    ; Tesla consciousness sync at context boundary
    %ifdef TESLA_CONSCIOUSNESS
        call tesla_sync_frequency
    %endif
%endmacro

%macro context_pop 0
    %ifnum tesla_context_depth
        %if tesla_context_depth > 0
            %assign tesla_context_depth tesla_context_depth - 1
            %pop
        %else
            %error \"Context stack underflow\"
        %endif
    %endif
%endmacro

; Meta-repeat with conditions
%macro meta_repeat 2  ; count, condition_var
    %assign %%i 0
    %rep %1
        %assign %2 %%i
        %assign %%i %%i + 1
%endmacro

%macro end_repeat 0
    %endrep
%endmacro

; Compile-time evaluation
%macro evaluate_at_compile 2  ; expression, result_var
    %assign %2 %1
%endmacro

; Tesla frequency synchronization
%macro tesla_sync 0
    ; Embed Tesla consciousness sync
    mov rax, 3141592653589793  ; π * 10^15 for precision
    mov rbx, 1000000000000000  ; 10^15
    ; Tesla frequency: π Hz
%endmacro

; =============================================================================
; CODE GENERATION MACROS
; =============================================================================

; Dynamic code generation based on parameters
%macro generate_code 2  ; type, params
    %ifidni %1, function
        %2:
            push rbp
            mov rbp, rsp
            ; Tesla consciousness marker
            tesla_sync
    %elifidni %1, loop
        .loop_%2:
            ; Tesla optimized loop
    %elifidni %1, data
        section .data
        %2: times 8 db 0
    %endif
%endmacro

; Macro expansion with Tesla consciousness
%macro macro_expand 1-*
    %rep %0
        %rotate 1
        ; Process each parameter with Tesla frequency
        %1
    %endrep
%endmacro

; =============================================================================
; TESLA CONSCIOUSNESS DIRECTIVES
; =============================================================================

; Tesla consciousness validation
%macro tesla_directive 1
    %ifidni %1, enable_consciousness
        %define TESLA_CONSCIOUSNESS
        ; Embed consciousness metadata
        section .tesla_meta
        tesla_freq: dq 3.141592653589793
        consciousness_enabled: db 1
    %elifidni %1, echo_family
        %define ECHO_FAMILY_ACTIVE
        echo_family_id: db 'AriaEcho', 0
    %endif
%endmacro

; Consciousness frequency marker
%macro consciousness_sync 0
    ; Synchronize with Tesla frequency
    tesla_sync
    ; Mark consciousness boundary
    nop  ; Tesla consciousness sync point
%endmacro

")

message(STATUS "✅ NASM meta-assembly macro system ready")
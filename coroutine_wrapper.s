.include "coroutines_config.inc"
.section .text
.global coroutine_wrapper
.extern coroutine_wrapper_
coroutine_wrapper:
    pushq %rbp
    movq %rsp, %rbp
    push %rdx
    movq $COROUTINE_STACK_SIZE, %rax
    inc %rdx
    mulq %rdx
    pop %rdx
    subq %rax, %rsp
    call coroutine_wrapper_
    movq %rbp, %rsp
    popq %rbp
    ret

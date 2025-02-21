.include "coroutines_config.inc"
.section .text
.global coroutine_wrapper
.extern coroutine_wrapper_
coroutine_wrapper:
    pushq %rbp                          # сохраняем указатель на базовый адрес фрейм стека
    movq %rsp, %rbp                     # сохраняем текущее значение указателя стека в указатель на базовый адрес
    push %rdx                           # вычисляем смещение адреса стека корутины (%rdx(3 параметр) + 1)*COROUTINE_STACK_SIZE
    movq $COROUTINE_STACK_SIZE, %rax
    inc %rdx
    mulq %rdx
    pop %rdx
    subq %rax, %rsp                     # вычитаем смещение стека корутины из текущего значения указателя стека
    call coroutine_wrapper_             # переходим к функции-обертке корутины
    movq %rbp, %rsp                     # восстанавливаем rsp и rbp
    popq %rbp
    ret

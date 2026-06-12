
#include <config.h>

.section .stack.trap, "aw", @nobits
    .align  2
    .globl  g_trapstackalloc
    .global g_trapstackbase
    .global g_top_trapstack
g_trapstackalloc:
g_trapstackbase:
    .space 768
g_top_trapstack:

    .align 2
    .globl g_trap_sp
    .type  g_trap_sp, object
g_trap_sp:
    .long 0
    .size g_trap_sp, .-g_trap_sp


/******************************************************************************
 * Functions:
 *     void trap(void);
 * default exception handler
 ******************************************************************************/
.text

    .align  2
    .global trap
    .type   trap, %function
trap:
    la      t0, g_trap_sp
    addi    t0, t0, -132
    sw      x1, 0(t0)
    sw      x2, 4(t0)
    sw      x3, 8(t0)
    sw      x4, 12(t0)
    sw      x6, 20(t0)
    sw      x7, 24(t0)
    sw      x8, 28(t0)
    sw      x9, 32(t0)
    sw      x10, 36(t0)
    sw      x11, 40(t0)
    sw      x12, 44(t0)
    sw      x13, 48(t0)
    sw      x14, 52(t0)
    sw      x15, 56(t0)
    sw      x16, 60(t0)
    sw      x17, 64(t0)
    sw      x18, 68(t0)
    sw      x19, 72(t0)
    sw      x20, 76(t0)
    sw      x21, 80(t0)
    sw      x22, 84(t0)
    sw      x23, 88(t0)
    sw      x24, 92(t0)
    sw      x25, 96(t0)
    sw      x26, 100(t0)
    sw      x27, 104(t0)
    sw      x28, 108(t0)
    sw      x29, 112(t0)
    sw      x30, 116(t0)
    sw      x31, 120(t0)
    csrr    a0, mepc
    sw      a0, 124(t0)
    csrr    a0, mstatus
    sw      a0, 128(t0)

    mv      a0, t0
    lw      t0, -4(sp)
    mv      sp, a0
    sw      t0, 16(sp)

    jal     trap_c


    .align  6
    .weak   default_handler
    .global default_handler
    .type   default_handler, %function
default_handler:
    /* Check for nmi */
    addi    sp, sp, -8
    sw      t0, 0x0(sp)
    sw      t1, 0x4(sp)
    csrr    t0, mcause
    andi    t0, t0, 0x3FF
    li      t1, 24
    beq     t0, t1, .nmi_handler
    lw      t0, 0x0(sp)
    lw      t1, 0x4(sp)
    addi    sp, sp, 8
    j      trap

.nmi_handler:
    lw      t0, 0x0(sp)
    lw      t1, 0x4(sp)
    addi    sp, sp, 8
    j       nmi_handler


    .size   default_handler, . - default_handler

/*    Macro to define default handlers. Default handler
 *    will be weak symbol and just dead loops. They can be
 *    overwritten by other handlers */
    .macro  def_irq_handler handler_name
    .weak   \handler_name
    .globl  \handler_name
    .set    \handler_name, default_handler
    .endm

    def_irq_handler tspend_handler
    def_irq_handler core_timer_irq_handler


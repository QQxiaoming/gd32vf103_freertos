#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include "n200_func.h"
#include "riscv_encoding.h"


typedef struct
{
	uint32_t zer0, ra, sp, gp, tp, t0, t1, t2, fp, s1, a0, a1, a2, a3, a4, a5, a6, a7, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, t3, t4, t5, t6, mstatus, mepc, msubm, mcause;
}exception_registers_t;

__attribute__((weak)) void	_exit(int __status)
{
    printf("application exit\n");
    for(;;);
}

__attribute__((weak)) uintptr_t handle_nmi(void)
{
    printf("nmi\n");
    _exit(1);
    return 0;
}

uintptr_t handle_trap(uintptr_t mcause, uintptr_t sp)
{
    if ((mcause&0xFFF) == 0xFFF)
    {
        handle_nmi();
    }
    else
    {
        exception_registers_t *regs = (exception_registers_t *)sp;
        printf("application trap\n");
        printf("----------------------------------\n");
        printf("ra\t = 0x%lx\n",regs->ra);
        printf("sp\t = 0x%lx\n",regs->sp);
        printf("t0\t = 0x%lx\n",regs->t0);
        printf("t1\t = 0x%lx\n",regs->t1);
        printf("t2\t = 0x%lx\n",regs->t2);
        printf("fp(s0)\t = 0x%lx\n",regs->fp);
        printf("s1\t = 0x%lx\n",regs->s1);
        printf("a0\t = 0x%lx\n",regs->a0);
        printf("a1\t = 0x%lx\n",regs->a1);
        printf("a2\t = 0x%lx\n",regs->a2);
        printf("a3\t = 0x%lx\n",regs->a3);
        printf("a4\t = 0x%lx\n",regs->a4);
        printf("a5\t = 0x%lx\n",regs->a5);
        printf("a6\t = 0x%lx\n",regs->a6);
        printf("a7\t = 0x%lx\n",regs->a7);
        printf("s2\t = 0x%lx\n",regs->s2);
        printf("s3\t = 0x%lx\n",regs->s3);
        printf("s4\t = 0x%lx\n",regs->s4);
        printf("s5\t = 0x%lx\n",regs->s5);
        printf("s6\t = 0x%lx\n",regs->s6);
        printf("s7\t = 0x%lx\n",regs->s7);
        printf("s8\t = 0x%lx\n",regs->s8);
        printf("s9\t = 0x%lx\n",regs->s9);
        printf("s10\t = 0x%lx\n",regs->s10);
        printf("s11\t = 0x%lx\n",regs->s11);
        printf("t3\t = 0x%lx\n",regs->t3);
        printf("t4\t = 0x%lx\n",regs->t4);
        printf("t5\t = 0x%lx\n",regs->t5);
        printf("t6\t = 0x%lx\n",regs->t6);
        printf("mstatus\t = 0x%lx\n",regs->mstatus);
        printf("msubm\t = 0x%lx\n",regs->msubm);
        printf("mepc\t = 0x%lx\n",regs->mepc);
        printf("mcause\t = 0x%lx\n",regs->mcause);
        printf("mdcause\t = 0x%lx\n",read_csr(0x7c9));//mdcause
        printf("mtval\t = 0x%lx\n",read_csr(mbadaddr));
        printf("----------------------------------\n");
		_exit(mcause);
    }
    return 0;
}

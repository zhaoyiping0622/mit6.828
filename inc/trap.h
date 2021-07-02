#ifndef JOS_INC_TRAP_H
#define JOS_INC_TRAP_H

// Trap numbers
// These are processor defined:
#define T_DIVIDE     0		// divide error
#define T_DEBUG      1		// debug exception
#define T_NMI        2		// non-maskable interrupt
#define T_BRKPT      3		// breakpoint
#define T_OFLOW      4		// overflow
#define T_BOUND      5		// bounds check
#define T_ILLOP      6		// illegal opcode
#define T_DEVICE     7		// device not available
#define T_DBLFLT     8		// double fault
/* #define T_COPROC  9 */	// reserved (not generated by recent processors)
#define T_TSS       10		// invalid task switch segment
#define T_SEGNP     11		// segment not present
#define T_STACK     12		// stack exception
#define T_GPFLT     13		// general protection fault
#define T_PGFLT     14		// page fault
/* #define T_RES    15 */	// reserved
#define T_FPERR     16		// floating point error
#define T_ALIGN     17		// aligment check
#define T_MCHK      18		// machine check
#define T_SIMDERR   19		// SIMD floating point error

// These are arbitrarily chosen, but with care not to overlap
// processor defined exceptions or interrupt vectors.
#define T_SYSCALL   48		// system call
#define T_DEFAULT   500		// catchall

#define IRQ_OFFSET	32	// IRQ 0 corresponds to int IRQ_OFFSET

// Hardware IRQ numbers. We receive these as (IRQ_OFFSET+IRQ_WHATEVER)
#define IRQ_TIMER        0
#define IRQ_KBD          1
#define IRQ_SERIAL       4
#define IRQ_SPURIOUS     7
#define IRQ_IDE         14
#define IRQ_ERROR       19

// #define HANDLER_EXAMPLE(name, num, istrap, sel, dpl)

#define TRAPHANDLERLIST(handler, handler_noec) \
  handler_noec(divide_error_traphandler, T_DIVIDE, 0, GD_KT, 0)\
  handler_noec(debug_exceptions_traphandler, T_DEBUG, 0, GD_KT, 0)\
  /* FIXME: non-maskable interrupt */\
  handler_noec(breakpoint_traphandler, T_BRKPT, 0, GD_KT, 0)\
  handler_noec(overflow_traphandler, T_OFLOW, 0, GD_KT, 0)\
  handler_noec(bounds_check_traphandler, T_BOUND, 0, GD_KT, 0)\
  handler_noec(invalid_opcode_traphandler, T_ILLOP, 0, GD_KT, 0)\
  handler_noec(device_not_available_traphandler, T_DEVICE, 0, GD_KT, 0)\
  handler(double_fault_traphandler, T_DBLFLT, 0, GD_KT, 0)\
  handler(invalid_tss_traphandler, T_TSS, 0, GD_KT, 0)\
  handler(segment_not_present_traphandler, T_SEGNP, 0, GD_KT, 0)\
  handler(stack_exception_traphandler, T_STACK, 0, GD_KT, 0)\
  handler(general_protection_fault_traphandler, T_GPFLT, 0, GD_KT, 3)\
  handler(page_fault_traphandler, T_PGFLT, 0, GD_KT, 0)\
  handler(floating_point_error_traphandler, T_FPERR, 0, GD_KT, 0)\

#ifndef __ASSEMBLER__

#include <inc/types.h>

struct PushRegs {
	/* registers as pushed by pusha */
	uint32_t reg_edi;
	uint32_t reg_esi;
	uint32_t reg_ebp;
	uint32_t reg_oesp;		/* Useless */
	uint32_t reg_ebx;
	uint32_t reg_edx;
	uint32_t reg_ecx;
	uint32_t reg_eax;
} __attribute__((packed));

struct Trapframe {
	struct PushRegs tf_regs;
	uint16_t tf_es;
	uint16_t tf_padding1;
	uint16_t tf_ds;
	uint16_t tf_padding2;
	uint32_t tf_trapno;
	/* below here defined by x86 hardware */
	uint32_t tf_err;
	uintptr_t tf_eip;
	uint16_t tf_cs;
	uint16_t tf_padding3;
	uint32_t tf_eflags;
	/* below here only when crossing rings, such as from user to kernel */
	uintptr_t tf_esp;
	uint16_t tf_ss;
	uint16_t tf_padding4;
} __attribute__((packed));


#endif /* !__ASSEMBLER__ */

#endif /* !JOS_INC_TRAP_H */

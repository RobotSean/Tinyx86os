/**
 * 中断处理
 */
#include "cpu/irq.h"
#include "comm/cpu_instr.h"
#include "tools/log.h"

static void dump_core_regs (exception_frame_t * frame) {
    // 打印CPU寄存器相关内容
    uint32_t esp, ss;
    if (frame->cs & 0x7) {
        ss = frame->ds;
        esp = frame->esp;
    } else {
        ss = frame->ss3;
        esp = frame->esp3;
    }
    log_printf("IRQ: %d, error code: %d.", frame->num, frame->error_code);
    log_printf("CS: %d\nDS: %d\nES: %d\nSS: %d\nFS:%d\nGS:%d",
               frame->cs, frame->ds, frame->es, ss, frame->fs, frame->gs
    );
     log_printf("EAX:0x%x\n"
                "EBX:0x%x\n"
                "ECX:0x%x\n"
                "EDX:0x%x\n"
                "EDI:0x%x\n"
                "ESI:0x%x\n"
                "EBP:0x%x\n"
                "ESP:0x%x\n",
               frame->eax, frame->ebx, frame->ecx, frame->edx,
               frame->edi, frame->esi, frame->ebp, esp);
    log_printf("EIP:0x%x\nEFLAGS:0x%x\n", frame->eip, frame->eflags);
}

static void do_default_handler (exception_frame_t * frame, const char * message) {
    log_printf("--------------------------------");
    log_printf("IRQ/Exception happend: %s.", message);
    dump_core_regs(frame);
    
    // todo: 留等以后补充打印任务栈的内容

    log_printf("--------------------------------");
    for (;;) {
        hlt();
    }
}
void do_handler_unknown (exception_frame_t * frame) {
	do_default_handler(frame, "Unknown exception.");
}

void do_handler_divider(exception_frame_t * frame) {
	do_default_handler(frame, "Device Error.");
}

void do_handler_Debug(exception_frame_t * frame) {
	do_default_handler(frame, "Debug Exception");
}

void do_handler_NMI(exception_frame_t * frame) {
	do_default_handler(frame, "NMI Interrupt.");
}

void do_handler_breakpoint(exception_frame_t * frame) {
	do_default_handler(frame, "Breakpoint.");
}

void do_handler_overflow(exception_frame_t * frame) {
	do_default_handler(frame, "Overflow.");
}

void do_handler_bound_range(exception_frame_t * frame) {
	do_default_handler(frame, "BOUND Range Exceeded.");
}

void do_handler_invalid_opcode(exception_frame_t * frame) {
	do_default_handler(frame, "Invalid Opcode.");
}

void do_handler_device_unavailable(exception_frame_t * frame) {
	do_default_handler(frame, "Device Not Available.");
}

void do_handler_double_fault(exception_frame_t * frame) {
	do_default_handler(frame, "Double Fault.");
}

void do_handler_invalid_tss(exception_frame_t * frame) {
	do_default_handler(frame, "Invalid TSS");
}

void do_handler_segment_not_present(exception_frame_t * frame) {
	do_default_handler(frame, "Segment Not Present.");
}

void do_handler_stack_segment_fault(exception_frame_t * frame) {
	do_default_handler(frame, "Stack-Segment Fault.");
}

void do_handler_general_protection(exception_frame_t * frame) {
    log_printf("--------------------------------");
    log_printf("IRQ/Exception happend: General Protection.");
    if (frame->error_code & ERR_EXT) {
        log_printf("the exception occurred during delivery of an "
                "event external to the program, such as an interrupt"
                "or an earlier exception.");
    } else {
        log_printf("the exception occurred during delivery of a"
                    "software interrupt (INT n, INT3, or INTO).");
    }
    
    if (frame->error_code & ERR_IDT) {
        log_printf("the index portion of the error code refers "
                    "to a gate descriptor in the IDT");
    } else {
        log_printf("the index refers to a descriptor in the GDT");
    }
    
    log_printf("segment index: %d", frame->error_code & 0xFFF8);

    dump_core_regs(frame);
    while (1) {
        hlt();
    }	
}

void do_handler_page_fault(exception_frame_t * frame) {
    log_printf("--------------------------------");
    log_printf("IRQ/Exception happend: Page fault.");
    if (frame->error_code & ERR_PAGE_P) {
        log_printf("\tpage-level protection violation: 0x%x.", read_cr2());
    } else {
        log_printf("\tPage doesn't present 0x%x", read_cr2());
    }
    
    if (frame->error_code & ERR_PAGE_WR) {
        log_printf("\tThe access causing the fault was a read.");
    } else {
        log_printf("\tThe access causing the fault was a write.");
    }
    
    if (frame->error_code & ERR_PAGE_US) {
        log_printf("\tA supervisor-mode access caused the fault.");
    } else {
        log_printf("\tA user-mode access caused the fault.");
    }

    dump_core_regs(frame);
    while (1) {
        hlt();
    }
}

void do_handler_fpu_error(exception_frame_t * frame) {
	do_default_handler(frame, "X87 FPU Floating Point Error.");
}

void do_handler_alignment_check(exception_frame_t * frame) {
	do_default_handler(frame, "Alignment Check.");
}

void do_handler_machine_check(exception_frame_t * frame) {
	do_default_handler(frame, "Machine Check.");
}

void do_handler_smd_exception(exception_frame_t * frame) {
	do_default_handler(frame, "SIMD Floating Point Exception.");
}

void do_handler_virtual_exception(exception_frame_t * frame) {
	do_default_handler(frame, "Virtualization Exception.");
}


void pic_send_eoi(int irq_num) {
    irq_num -= IRQ_PIC_START;

    // 从片也可能需要发送EOI
    if (irq_num >= 8) {
        outb(PIC1_OCW2, PIC_OCW2_EOI);
    }

    outb(PIC0_OCW2, PIC_OCW2_EOI);
}

void irq_enable(int irq_num) {
    if (irq_num < IRQ_PIC_START) {
        return;
    }

    irq_num -= IRQ_PIC_START;
    if (irq_num < 8) {
        uint8_t mask = inb(PIC0_IMR) & ~(1 << irq_num);
        outb(PIC0_IMR, mask);
    } else {
        irq_num -= 8;
        uint8_t mask = inb(PIC1_IMR) & ~(1 << irq_num);
        outb(PIC1_IMR, mask);
    }
}

void irq_disable(int irq_num) {
    if (irq_num < IRQ_PIC_START) {
        return;
    }

    irq_num -= IRQ_PIC_START;
    if (irq_num < 8) {
        uint8_t mask = inb(PIC0_IMR) | (1 << irq_num);
        outb(PIC0_IMR, mask);
    } else {
        irq_num -= 8;
        uint8_t mask = inb(PIC1_IMR) | (1 << irq_num);
        outb(PIC1_IMR, mask);
    }
}

void irq_disable_global(void) {
    cli();
}

void irq_enable_global(void) {
    sti();
}

/**
 * @brief 进入中断保护
 */
irq_state_t irq_enter_protection (void) {
    irq_state_t state = read_eflags();
    irq_disable_global();
    return state;
}

/**
 * @brief 退出中断保护
 */
void irq_leave_protection (irq_state_t state) {
    //如果irq_enter_protection是关闭的，我们退出时不应该将中断打开
    write_eflags(state);
}

/**
 * CPU设置：设置并加载GDT表，设置并加载IDT表
 */
#include "comm/cpu_instr.h"
#include "cpu/cpu.h"
#include "os_cfg.h"
#include "cpu/irq.h"


static segment_desc_t gdt_table[GDT_TABLE_SIZE];
static gate_desc_t idt_table[IDT_TABLE_NR];	// 中断描述表


/**
 * 设置段描述符
 */
void segment_desc_set(int selector, uint32_t base, uint32_t limit, uint16_t attr) {
    segment_desc_t * desc = gdt_table + (selector >> 3);

	// 如果界限比较长，将长度单位换成4KB
	if (limit > 0xfffff) {
		attr |= 0x8000;
		limit /= 0x1000;
	}
	desc->limit15_0 = limit & 0xffff;
	desc->base15_0 = base & 0xffff;
	desc->base23_16 = (base >> 16) & 0xff;
	desc->attr = attr | (((limit >> 16) & 0xf) << 8);
	desc->base31_24 = (base >> 24) & 0xff;
}

/**
 * 设置门描述符
 */
void gate_desc_set(gate_desc_t * desc, uint16_t selector, uint32_t offset, uint16_t attr) {
	desc->offset15_0 = offset & 0xffff;
	desc->selector = selector;
	desc->attr = attr;
	desc->offset31_16 = (offset >> 16) & 0xffff;
}



/**
 * 初始化GDT
 */
void init_gdt(void) {
	// 全部清空
    for (int i = 0; i < GDT_TABLE_SIZE; i++) {
        segment_desc_set(i << 3, 0, 0, 0);
    }

    //数据段
    segment_desc_set(KERNEL_SELECTOR_DS, 0x00000000, 0xFFFFFFFF,
                     SEG_P_PRESENT | SEG_DPL0 | SEG_S_NORMAL | SEG_TYPE_DATA
                     | SEG_TYPE_RW | SEG_D | SEG_G);

    // 只能用非一致代码段，以便通过调用门更改当前任务的CPL执行关键的资源访问操作
    segment_desc_set(KERNEL_SELECTOR_CS, 0x00000000, 0xFFFFFFFF,
                     SEG_P_PRESENT | SEG_DPL0 | SEG_S_NORMAL | SEG_TYPE_CODE
                     | SEG_TYPE_RW | SEG_D | SEG_G);


    // 加载gdt
    lgdt((uint32_t)gdt_table, sizeof(gdt_table));
}



/**
 * @brief 安装中断或异常处理程序
 */
int irq_install(int irq_num, irq_handler_t handler) {
	if (irq_num >= IDT_TABLE_NR) {
		return -1;
	}
    gate_desc_set(idt_table + irq_num, KERNEL_SELECTOR_CS, (uint32_t) handler,
                  GATE_P_PRESENT | GATE_DPL0 | GATE_TYPE_IDT);
	return 0;
}


/**
 * @brief 中断和异常初始化
 */
void irq_init(void) {	
	for (uint32_t i = 0; i < IDT_TABLE_NR; i++) {
        //不能直接放中断处理函数的地址，因为函数结束调用后需要，iret返回，需要汇编处理
    	gate_desc_set(idt_table + i, KERNEL_SELECTOR_CS, (uint32_t) exception_handler_unknown,
                  GATE_P_PRESENT | GATE_DPL0 | GATE_TYPE_IDT);
	}

    // 设置异常处理接口
	irq_install(IRQ0_DE, exception_handler_divider);
	irq_install(IRQ1_DB, exception_handler_Debug);
	irq_install(IRQ2_NMI, exception_handler_NMI);
	irq_install(IRQ3_BP, exception_handler_breakpoint);
	irq_install(IRQ4_OF, exception_handler_overflow);
	irq_install(IRQ5_BR, exception_handler_bound_range);
	irq_install(IRQ6_UD, exception_handler_invalid_opcode);
	irq_install(IRQ7_NM, exception_handler_device_unavailable);
	irq_install(IRQ8_DF, exception_handler_double_fault);
	irq_install(IRQ10_TS, exception_handler_invalid_tss);
	irq_install(IRQ11_NP, exception_handler_segment_not_present);
	irq_install(IRQ12_SS, exception_handler_stack_segment_fault);
	irq_install(IRQ13_GP, exception_handler_general_protection);
	irq_install(IRQ14_PF, exception_handler_page_fault);
	irq_install(IRQ16_MF, exception_handler_fpu_error);
	irq_install(IRQ17_AC, exception_handler_alignment_check);
	irq_install(IRQ18_MC, exception_handler_machine_check);
	irq_install(IRQ19_XM, exception_handler_smd_exception);
	irq_install(IRQ20_VE, exception_handler_virtual_exception);

	lidt((uint32_t)idt_table, sizeof(idt_table));
}

/**
 * CPU初始化
 */
void cpu_init (void) {
    init_gdt();
    irq_init();
}




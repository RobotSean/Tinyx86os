/**
 * 自己动手写操作系统
 * 然后加载内核，并跳转至内核运行
 *
 */
 #include "os_cfg.h"
  	// 不必加.code32因默认就是32位
 	.text
 	.global _start
	.extern kernel_init
	.extern init_main
_start:
	# 第一种方法
    push %ebp
    mov %esp, %ebp
    mov 0x8(%ebp), %eax
    push %eax
	
	call kernel_init


	jmp $KERNEL_SELECTOR_CS, $gdt_reload

gdt_reload:
	mov $KERNEL_SELECTOR_DS, %ax		// 16为数据段选择子
	mov %ax, %ds
    mov %ax, %ss
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

	// 栈设置
	mov $(stack + KERNEL_STACK_SIZE), %esp

	// 栈和段等沿用之前的设置
	jmp init_main

	.bss
.comm stack, KERNEL_STACK_SIZE    // comm 声明未初始化的通用内存区域，以字节计





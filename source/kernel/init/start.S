/**
 * 自己动手写操作系统
 * 然后加载内核，并跳转至内核运行
 *
 */
  	// 不必加.code32因默认就是32位
 	.text
 	.global _start
	.extern kernel_init
_start:
	# 第一种方法
    push %ebp
    mov %esp, %ebp
    mov 0x8(%ebp), %eax
    push %eax
	
	call kernel_init

	jmp .





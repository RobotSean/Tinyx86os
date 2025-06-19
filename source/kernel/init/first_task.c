/**
 * 内核初始化以及测试代码
 */
#include "applib/lib_syscall.h"
int first_task_main (void) {
    for (;;) {
        // 特权模式3，没有权限访问权限级0的操作系统代码
        // log_printf("first task.");
        // sys_msleep(1000);
        msleep(1000);
    }

    return 0;
} 
/**
 * 内核初始化以及测试代码
 */
#include "comm/boot_info.h"
#include "comm/cpu_instr.h"
#include "cpu/cpu.h"
#include "os_cfg.h"
#include "dev/time.h"
#include "cpu/irq.h"
#include "tools/log.h"
#include "tools/klib.h"
#include "core/task.h"
static boot_info_t * init_boot_info;        // 启动信息
/**
 * 内核入口
 */
void kernel_init (boot_info_t * boot_info) {
    init_boot_info = boot_info;
    // 初始化CPU:设置GDT、IDT
    cpu_init();
    log_init();
    time_init();
}


static task_t first_task;       // 第一个任务
static task_t init_task;
static uint32_t init_task_stack[1024];	// 空闲任务堆栈

/**
 * 初始任务函数
 * 目前暂时用函数表示，以后将会作为加载为进程
 */
void init_task_entry(void) {
    int count = 0;

    for (;;) {
        log_printf("init task: %d", count++);
        task_switch_from_to(&init_task, &first_task);
    }
}


void init_main(void) {
    // int a = 3 / 0;  //测试异常
    //irq_enable_global(); //测试定时中断

    //测试日志
    log_printf("Kernel is running....");
    log_printf("Version: %s, name: %s", OS_VERSION, "tiny x86 os");
    log_printf("%d %d %x %c", -123, 123456, 0x12345, 'a');

    //测试断言
    // int a = 3;
    // ASSERT(a > 2);
    // ASSERT(a < 2);


    task_init(&init_task, (uint32_t)init_task_entry, (uint32_t)&init_task_stack[1024]);
    //eip 和 esp 没有被设置到TSS段中，只为其设置了TSS段描述符，TSS段描述符指向TSS段
    //当切换的时候硬件会自动保存TSS
    task_init(&first_task, 0, 0);
    //设置tr寄存器指向当前任务的 TSS段描述符
    write_tr(first_task.tss_sel);

    int count = 0;
    for (;;) {
        log_printf("first task: %d", count++);
        task_switch_from_to(&first_task, &init_task);
    }

}

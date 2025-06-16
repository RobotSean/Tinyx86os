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
#include "tools/list.h"
#include "ipc/sem.h"
#include "core/memory.h"
static boot_info_t * init_boot_info;        // 启动信息
/**
 * 内核入口
 */
void kernel_init (boot_info_t * boot_info) {
    init_boot_info = boot_info;
    // 初始化CPU:设置GDT、IDT
    cpu_init();
    // 内存初始化要放前面一点，因为后面的代码可能需要内存分配
    memory_init(boot_info);

    log_init();
    time_init();

    task_manager_init();
}



static task_t init_task;
static uint32_t init_task_stack[1024];	// 空闲任务堆栈
static sem_t sem_1;
static sem_t sem_2;

int count = 0;

/**
 * 初始任务函数
 * 目前暂时用函数表示，以后将会作为加载为进程
 */
void init_task_entry(void) {
    for (;;) {
        sem_wait(&sem_1);
        log_printf("init task : %d", count++);
        sem_notify(&sem_2);
    }
}


void init_main(void) {
    // int a = 3 / 0;  //测试异常
    //irq_enable_global(); //测试定时中断

    //测试日志
    log_printf("Kernel is running....");
    log_printf("Version: %s, name: %s", OS_VERSION, "tiny x86 os");
    log_printf("%d %d %x %c", -123, 123456, 0x12345, 'a');


    task_first_init();
    task_init(&init_task, "init task", (uint32_t)init_task_entry, (uint32_t)&init_task_stack[1024]);
    

    
    // 放在开中断前，以避免定时中断切换至其它任务，而此时信号量还未初始化
    sem_init(&sem_1, 0);
    sem_init(&sem_2, 0);
    irq_enable_global();


    for (;;) {
        sem_notify(&sem_1);
        log_printf("first task: %d", count++);
        sem_wait(&sem_2);  
    }

}

/**
 * 中断处理
 */
#include "cpu/irq.h"


static void do_default_handler (const char * message) {
    for (;;) {}
}

void do_handler_unknown (void) {
	do_default_handler("Unknown exception.");
}



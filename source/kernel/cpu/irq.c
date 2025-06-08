/**
 * 中断处理
 */
#include "cpu/irq.h"

static void do_default_handler (exception_frame_t * frame, const char * message) {
    for (;;) {}
}

void do_handler_unknown (exception_frame_t * frame) {
	do_default_handler(frame, "Unknown exception.");
}

void do_handler_divider(exception_frame_t * frame) {
	do_default_handler(frame, "Device Error.");
}


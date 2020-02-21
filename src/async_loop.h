#ifndef ASYNC_LOOP_H
#define ASYNC_LOOP_H

#include "context.h"

void async_loop_init(app_context_t* ctx);
void* async_loop_main(void* context);
void async_loop_wait_before_main_loop(app_context_t* ctx);
void async_loop_wakeup(app_context_t* ctx);

#endif

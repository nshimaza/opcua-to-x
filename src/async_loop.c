#include <assert.h>
#include <uv.h>

#include "async_loop.h"
#include "log.h"
#include "mvar.h"

void
do_job(uv_async_t* handle) {
    ULTRACE("do_job() is not yet implemented.");
}

void
async_loop_init(app_context_t* ctx) {
    init_mvar_unit(&ctx->ready_mark);
    int err = uv_async_init(uv_default_loop(), &ctx->wakeup, do_job);
    if (err != 0) {
        UVERR("complink_context_init: uv_async_init", err);
    }
    assert(err == 0);
}

void*
async_loop_main(void* context) {
    app_context_t* ctx = context;
    put_mvar(&ctx->ready_mark, NULL);
    uv_run(uv_default_loop(), UV_RUN_DEFAULT);
    ULTRACE("Asynchronous networking loop finished.");
    return NULL;
}

void
async_loop_wait_before_main_loop(app_context_t* ctx) {
    read_mvar(NULL, &ctx->ready_mark);
}

void
async_loop_wakeup(app_context_t* ctx) {
    int err = uv_async_send(&ctx->wakeup);
    assert(err == 0);
}

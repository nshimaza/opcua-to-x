#ifndef CONTEXT_H
#define CONTEXT_H

#include <stddef.h>
#include <stdint.h>
#include <uv.h>

#include "mvar.h"

#define MAX_DEVICES 2
#define MAX_ROBOTS 4
#define MAX_AXES 6

typedef struct {
    uint32_t s_addr;
    uint16_t port;
} device_conf_t;

typedef struct {
    device_conf_t plc;
    device_conf_t robot;
} config_t;

typedef struct {
    size_t ns_di;
    size_t ns_plc;
    size_t ns_robot;
} namespace_index_t;

typedef struct {
    uv_async_t wakeup;
    namespace_index_t ns;
    mvar_abs_t ready_mark;
    config_t conf;
} app_context_t;

#endif

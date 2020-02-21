#include <open62541/plugin/log_stdout.h>

#define TOSTR(n) TOSTR_(n)
#define TOSTR_(n) #n

#define UVERR(f, e) (UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, \
    __FILE__ ":" TOSTR(__LINE__) ": %s failed with %s: %s", (f), uv_err_name(e), uv_strerror(e)))
#define SYSERR(f, e) (UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, \
    __FILE__ ":" TOSTR(__LINE__) ": %s failed with errno %d: %s", (f), (e), uv_strerror(e)))
#define SVERR(f, e) (UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, \
    __FILE__ ":" TOSTR(__LINE__) ": %s failed with %s", (f), UA_StatusCode_name(e)))
#define ULERR(...) (UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, __VA_ARGS__))

#define ULINFO(...) (UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, __VA_ARGS__))

#define SVTRACE(...) (UA_LOG_TRACE(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, \
    __FILE__ ":" TOSTR(__LINE__) ": " __VA_ARGS__))
#define ULTRACE(...) (UA_LOG_TRACE(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, \
    __FILE__ ":" TOSTR(__LINE__) ": " __VA_ARGS__))

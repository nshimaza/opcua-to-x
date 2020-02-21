#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include <open62541/plugin/log_stdout.h>
#include <open62541/server.h>
#include <open62541/server_config_default.h>

#include <uv.h>
#include <ini.h>

#include "open62541/namespace_di_generated.h"
#include "open62541/namespace_plc_generated.h"
#include "open62541/namespace_robot_generated.h"

#include "async_loop.h"
#include "context.h"
#include "log.h"
#include "robot.h"

#include "util.h"

/**
 * Callback function for ini_parse()
 *
 * @param user      Pointer to config_t.
 * @param section   Current section name.
 * @param name      Parsed variable name.
 * @param value     Parsed variable value.
 *
 * This parser understands section "[robot]" and "[plc]".
 * It expects following parameters in each section.
 *
 * device_ip: <ipv4 address of device in number dot notation>
 * device_port: <listening port number of the device in decimal>
 *
 * This configuration reader uses inih package from Ben Hoyt (benhoyt).
 * https://github.com/benhoyt/inih
 */
static int
read_config_handler(void* user, const char* section, const char* name, const char* value) {
    config_t* out_conf = user;
    if (*section == '\0') {
        ULERR("Config error: Section must be specified.");
        return 0;
    }
    device_conf_t* target;
    if (strncmp("robot", section, INI_MAX_LINE) == 0) {
        target = &out_conf->robot;
    } else if (strncmp("plc", section, INI_MAX_LINE) == 0) {
        target = &out_conf->plc;
    } else {
        ULERR("Config error: Unknown section name.");
        return 0;
    }
    ULTRACE("read_config_handler: found %s = %s in section %s", name, value, section);
    if (strncmp("device_ip", name, INI_MAX_LINE) == 0) {
        unsigned int d1, d2, d3, d4;
        if (sscanf(value, "%u.%u.%u.%u", &d1, &d2, &d3, &d4) != 4) {
            ULERR("Config error: Value of device_ip must be a valid IPv4 address in number dot notation.");
            return 0;
        }
        if (255 < d1 || 255 < d2 || 255 < d3 || 255 < d4) {
            ULERR("Config error: Every decimal separated by dot can't exceed 255.");
            return 0;
        }
        target->s_addr = htonl(d1 << 24 | d2 << 16 | d3 << 8 | d4);
        ULTRACE("read_config_handler: set target->s_addr to 0x%08x", ntohl(target->s_addr));
    } else if (strncmp("device_port", name, INI_MAX_LINE) == 0) {
        unsigned short port;
        if (sscanf(value, "%hu", &port) != 1) {
            ULERR("Config error: Value of controller_port must be a valid port number in decimal.");
            return 0;
        }
        target->port = htons(port);
        ULTRACE("read_config_handler: set target->port to %hd", ntohs(target->s_addr));
    } else {
        ULERR("Config error: Unknown parameter %s.", name);
        return 0;
    }
    return 1;
}

static void
dump_config(const config_t* const conf) {
    ULINFO("plc ip addr = %d.%d.%d.%d, port = %d",
        ntohl(conf->plc.s_addr) >> 24, ntohl(conf->plc.s_addr) >> 16 & 0xff,
        ntohl(conf->plc.s_addr) >> 8 & 0xff, ntohl(conf->plc.s_addr) & 0xff,
        ntohs(conf->plc.port));
    ULINFO("robot ip addr = %d.%d.%d.%d, port = %d",
        ntohl(conf->robot.s_addr) >> 24, ntohl(conf->robot.s_addr) >> 16 & 0xff,
        ntohl(conf->robot.s_addr) >> 8 & 0xff, ntohl(conf->robot.s_addr) & 0xff,
        ntohs(conf->robot.port));
}

/**
 * Read configuration
 *
 * Read configuration from .ini file dedignaged via given environment variable.
 *
 * @param out_conf  Pointer to config_t where read configurations are output.
 * Content of out_conf must be initialized with zero before this function is
 * called.
 * @param env       Environment variable name which designate configuration file
 * path.
 */
static int
read_config(config_t* const out_conf, const char* const env) {
    char* config_file = getenv(env);
    if (config_file == NULL) {
        ULTRACE("Environment variable %s not defined.", env);
        return 1;
    }
    ULTRACE("read_config: Configuration file name is %s.  Trying open and parse it.", config_file);
    if (ini_parse(config_file, read_config_handler, out_conf) < 0) {
        ULERR("Opening configration file %s failed.", config_file);
        // UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Opening configration file %s failed.", config_file);
        return 1;
    }
    dump_config(out_conf);
    return 0;
}

/**
 * Instantiate companion namespaces
 *
 * Instantiate companion namespaces such as DI, PLCopen, and Robotics.  Output
 * dynamically assigned namespace indics.
 *
 * @param server Pointer to UA_sServer instance where namespaces will be
 * instantiated.
 * @param out_ns Pointer to namespace_index_t where namespace indics for DI,
 * PLCopen, and Robotics will be written.  Namespace indics are dynamically
 * assigned by framework and output to here.
 */
static void
setup_companion_namespaces(UA_Server* server, namespace_index_t* out_ns) {
    /* create nodes from nodesets */
    UA_StatusCode err = namespace_di_generated(server);
    assert(err == UA_STATUSCODE_GOOD);
    err = namespace_plc_generated(server);
    assert(err == UA_STATUSCODE_GOOD);
    err = namespace_robot_generated(server);
    assert(err == UA_STATUSCODE_GOOD);

    /* Get namespace indices of companion specifications. */
    static const UA_String di_url = UA_STRING_STATIC("http://opcfoundation.org/UA/DI/");
    err = UA_Server_getNamespaceByName(server, di_url, &out_ns->ns_di);
    assert(err == UA_STATUSCODE_GOOD);
    static const UA_String plc_url = UA_STRING_STATIC("http://PLCopen.org/OpcUa/IEC61131-3/");
    err = UA_Server_getNamespaceByName(server, plc_url, &out_ns->ns_plc);
    assert(err == UA_STATUSCODE_GOOD);
    static const UA_String robot_url = UA_STRING_STATIC("http://opcfoundation.org/UA/Robotics/");
    err = UA_Server_getNamespaceByName(server, robot_url, &out_ns->ns_robot);
    assert(err == UA_STATUSCODE_GOOD);
}

static volatile UA_Boolean running = true;
static void
stop_handler(int sig) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "received ctrl-c");
    running = false;
}

int
main(const int argc, const char* const argv[]) {
    int exit_status = EXIT_FAILURE;

    if (argc != 2) {
        fprintf(stderr, "Usage: opcua-to-x ENV_OF_CONFIG_PATH\n");
        goto abort_no_resources;
    }

    static app_context_t ctx = {
        .conf.plc = {
            .s_addr = 0,
            .port = 0
        },
        .conf.robot = {
            .s_addr = 0,
            .port = 0
        }
    };

    if (read_config(&ctx.conf, argv[1]) != 0) {
        ULERR("Read configuration failed.  Aborting.");
        goto abort_no_resources;
    }

    async_loop_init(&ctx);
    static pthread_t async_loop_thread;
    pthread_create(&async_loop_thread, NULL, async_loop_main, &ctx);
    async_loop_wait_before_main_loop(&ctx);

    signal(SIGINT, stop_handler);
    signal(SIGTERM, stop_handler);

    UA_Server* server = UA_Server_new();
    UA_ServerConfig_setDefault(UA_Server_getConfig(server));
    UA_ServerConfig *config = UA_Server_getConfig(server);
    UA_ServerConfig_setDefault(UA_Server_getConfig(server));
    config->verifyRequestTimestamp = UA_RULEHANDLING_WARN;

    setup_companion_namespaces(server, &ctx.ns);
    SVTRACE("Namespace indice: di = %ld, plc = %ld, robot = %ld", ctx.ns.ns_di, ctx.ns.ns_plc, ctx.ns.ns_robot);

    instantiate_robot_rest_nodes(server, &ctx);

    // addCtrlConfiguration(server, ns);

    if (UA_Server_run(server, &running) == UA_STATUSCODE_GOOD) {
        exit_status = EXIT_SUCCESS;
    }

abort_server:
    UA_LOG_TRACE(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Shutting down server.");
    UA_Server_delete(server);
abort_async_loop_thread:
    UA_LOG_TRACE(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Shutting down asynchronous networking thread.");
    uv_stop(uv_default_loop());
    async_loop_wakeup(&ctx);
    pthread_join(async_loop_thread, NULL);
abort_no_resources:
    UA_LOG_TRACE(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Exiting with status code %d.", exit_status);
    return exit_status;
}

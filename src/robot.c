#include <assert.h>
#include <open62541/server.h>

#include "context.h"
#include "robot.h"
#include "util.h"

void
instantiate_robot_rest_nodes(UA_Server *server, app_context_t* ctx) {
    /* Add MotionDeviceSystem object under DeviceSet */
    UA_ObjectAttributes attr = UA_ObjectAttributes_default;
    attr.displayName = UA_LOCALIZEDTEXT("en-US", "MotionDeviceSystem");
    UA_NodeId motionDeviceSystemNodeId;
    UA_StatusCode err = UA_Server_addObjectNode(server, UA_NODEID_NULL,
                            UA_NODEID_NUMERIC(ctx->ns.ns_di, 5001),    /* Parent is DeviceSet */
                            UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
                            UA_QUALIFIEDNAME(1, "MotionDeviceSystem"),
                            UA_NODEID_NUMERIC(ctx->ns.ns_robot, 1002), /* Type is MotionDeviceSystemType */
                            attr, NULL, &motionDeviceSystemNodeId);
    assert(err == UA_STATUSCODE_GOOD);
    /*
     * Lookup child FolderType object 'Controllers'.  Controllers is
     * automatically instantiated via data type definition of MotionDeviceSystem
     * node.
     */
    UA_NodeId collectorsNodeId;
    find_node_id(server, &collectorsNodeId, motionDeviceSystemNodeId,
        UA_QUALIFIEDNAME(ctx->ns.ns_robot, "Controllers"));
    /* Add a ControllerIdentifier object under Controllers folder. */
    attr = UA_ObjectAttributes_default;
    char controller_name[20];
    snprintf(controller_name, sizeof controller_name, "Controller");
    attr.displayName = UA_LOCALIZEDTEXT("en-US", controller_name);
    UA_NodeId ControllerIdNodeId;
    err = UA_Server_addObjectNode(server, UA_NODEID_NULL,
                                    collectorsNodeId,                           /* Parent is Controllers */
                                    UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                                    UA_QUALIFIEDNAME(1, controller_name),
                                    UA_NODEID_NUMERIC(ctx->ns.ns_robot, 1003),  /* Type is ControllerType */
                                    attr, NULL, &ControllerIdNodeId);
    assert(err == UA_STATUSCODE_GOOD);
    UA_LocalizedText manufacturer = UA_LOCALIZEDTEXT("en-US", "EXAMPLE Robotics Corp.");
    err = UA_Server_writeObjectProperty_scalar(server, ControllerIdNodeId,
        UA_QUALIFIEDNAME(ctx->ns.ns_di, "Manufacturer"), &manufacturer, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]);
    assert(err == UA_STATUSCODE_GOOD);
    UA_LocalizedText model = UA_LOCALIZEDTEXT("en-US", "ROBOT MASTER II");
    err = UA_Server_writeObjectProperty_scalar(server, ControllerIdNodeId,
        UA_QUALIFIEDNAME(ctx->ns.ns_di, "Model"), &model, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]);
    assert(err == UA_STATUSCODE_GOOD);
    UA_String serial = UA_STRING("ABC12345");
    err = UA_Server_writeObjectProperty_scalar(server, ControllerIdNodeId,
        UA_QUALIFIEDNAME(ctx->ns.ns_di, "SerialNumber"), &serial, &UA_TYPES[UA_TYPES_STRING]);
    assert(err == UA_STATUSCODE_GOOD);
    /*
        * Lookup child FolderType object 'Software' and 'MotionDevices'.  Software is automatically
        * instantiated via data type definition of ControllerIdentifier node.
        */
    UA_NodeId softwareNodeId;
    find_node_id(server, &softwareNodeId, ControllerIdNodeId, UA_QUALIFIEDNAME(ctx->ns.ns_robot, "Software"));
    /* Add SoftwareIdentifier objects under Software folder. */
    for (int i = 0; i < MAX_ROBOTS; i++) {
        char robot_name[20];
        snprintf(robot_name, sizeof robot_name, "Robot%d", i + 1);
        attr = UA_ObjectAttributes_default;
        attr.displayName = UA_LOCALIZEDTEXT("en-US", robot_name);
        UA_NodeId softwareIdNodeId;
        err = UA_Server_addObjectNode(server, UA_NODEID_NULL, softwareNodeId,
                                        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                                        UA_QUALIFIEDNAME(1, robot_name),
                                        UA_NODEID_NUMERIC(ctx->ns.ns_di, 15106),    // Type is SoftwareType
                                        attr, NULL, &softwareIdNodeId);
        assert(err == UA_STATUSCODE_GOOD);
        err = UA_Server_writeObjectProperty_scalar(server, softwareIdNodeId,
                UA_QUALIFIEDNAME(ctx->ns.ns_di, "Manufacturer"), &manufacturer, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]);
        assert(err == UA_STATUSCODE_GOOD);
        UA_LocalizedText model = UA_LOCALIZEDTEXT("en-US", "Robot TYPE III");
        err = UA_Server_writeObjectProperty_scalar(server, softwareIdNodeId,
                UA_QUALIFIEDNAME(ctx->ns.ns_di, "Model"), &model, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]);
        assert(err == UA_STATUSCODE_GOOD);
        UA_String rev = UA_STRING("Revision 1.0.0");
        err = UA_Server_writeObjectProperty_scalar(server, softwareIdNodeId,
                UA_QUALIFIEDNAME(ctx->ns.ns_di, "SoftwareRevision"), &rev, &UA_TYPES[UA_TYPES_STRING]);
        assert(err == UA_STATUSCODE_GOOD);
    }
    /*
     * Lookup child FolderType object 'MotionDevices'.  MotionDevices is
     * automatically instantiated via data type definition of MotionDeviceSystem
     * node.
     */
    UA_NodeId motionDevicesNodeId;
    find_node_id(server, &motionDevicesNodeId, motionDeviceSystemNodeId,
        UA_QUALIFIEDNAME(ctx->ns.ns_robot, "MotionDevices"));
    for (int i = 0; i < MAX_ROBOTS; i++) {
        char robot_name[20];
        snprintf(robot_name, sizeof robot_name, "Robot%d", i + 1);
        attr = UA_ObjectAttributes_default;
        attr.displayName = UA_LOCALIZEDTEXT("en-US", robot_name);
        UA_NodeId motionDeviceNodeId;
        err = UA_Server_addObjectNode(server, UA_NODEID_NULL,
                                        motionDevicesNodeId,
                                        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                                        UA_QUALIFIEDNAME(1, robot_name),
                                        UA_NODEID_NUMERIC(ctx->ns.ns_robot, 1004),  // Type is MotionDeviceType
                                        attr, NULL, &motionDeviceNodeId);
        err = UA_Server_writeObjectProperty_scalar(server, motionDeviceNodeId,
            UA_QUALIFIEDNAME(ctx->ns.ns_di, "Manufacturer"), &manufacturer, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]);
        assert(err == UA_STATUSCODE_GOOD);
        err = UA_Server_writeObjectProperty_scalar(server, motionDeviceNodeId,
                UA_QUALIFIEDNAME(ctx->ns.ns_di, "Model"), &model, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]);
        assert(err == UA_STATUSCODE_GOOD);
        char sn_str[20];
        snprintf(sn_str, sizeof sn_str, "XYZ987%d", i);
        UA_String serial = UA_STRING(sn_str);
        err = UA_Server_writeObjectProperty_scalar(server, motionDeviceNodeId,
            UA_QUALIFIEDNAME(ctx->ns.ns_di, "SerialNumber"), &serial, &UA_TYPES[UA_TYPES_STRING]);
        assert(err == UA_STATUSCODE_GOOD);
        /*
         * Lookup child FolderType object 'Axes'.  Axes is automatically
         * instantiated via data type definition of MotionDevice node.
         */
        UA_NodeId axesNodeId;
        find_node_id(server, &axesNodeId, motionDeviceNodeId, UA_QUALIFIEDNAME(ctx->ns.ns_robot, "Axes"));
        for (int k = 0; k < MAX_AXES; k++) {
            char axis_name[20];
            snprintf(axis_name, sizeof axis_name, "Axis%d", k + 1);
            attr = UA_ObjectAttributes_default;
            attr.displayName = UA_LOCALIZEDTEXT("en-US", axis_name);
            UA_NodeId axisNodeId;
            err = UA_Server_addObjectNode(server, UA_NODEID_NULL,
                                            axesNodeId,
                                            UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                                            UA_QUALIFIEDNAME(1, axis_name),
                                            UA_NODEID_NUMERIC(ctx->ns.ns_robot, 16601),  // Type is AxisType
                                            attr, NULL, &axisNodeId);
        }
    }
}

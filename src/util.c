#include <assert.h>
#include <open62541/plugin/log_stdout.h>
#include <open62541/server.h>
#include <open62541/server_config_default.h>

#include "util.h"

void
show_ua_string(char* out_str, size_t out_str_len, const UA_String src) {
    if (out_str_len == 0) {
        return;
    }
    if (src.length < out_str_len) {
        memcpy(out_str, src.data, src.length);
        out_str[src.length] = '\0';
        return;
    }
    memcpy(out_str, src.data, out_str_len - 1);
    out_str[out_str_len - 1] = '\0';
}

int
show_node_id(char* out_str, size_t out_str_len, const UA_NodeId id) {
    if (out_str_len == 0) {
        return 0;
    }
    static const char string_type[] = "(string) ";
    const UA_Guid* guid;
    switch (id.identifierType) {
        case UA_NODEIDTYPE_NUMERIC:
        case 1:
        case 2:
            return snprintf(out_str, out_str_len, "(numeric) %u", id.identifier.numeric);

        case UA_NODEIDTYPE_STRING:
            {
                char str[200];
                show_ua_string(str, sizeof str, id.identifier.string);
                return snprintf(out_str, out_str_len, "(string) %s", str);
            }

        case UA_NODEIDTYPE_GUID:
            guid = &id.identifier.guid;
            return snprintf(out_str, out_str_len, "(GUID) %08x-%04x-%04x-%01x%01x%01x%01x%01x%01x%01x%01x",
                guid->data1, guid->data2, guid->data3,
                guid->data4[0], guid->data4[1], guid->data4[2], guid->data4[3],
                guid->data4[4], guid->data4[5], guid->data4[6], guid->data4[7]);

        case UA_NODEIDTYPE_BYTESTRING:
            return snprintf(out_str, out_str_len, "(byteString) <omit>");
    }
}

void
find_node_id(UA_Server *server, UA_NodeId* out_node_id, const UA_NodeId start_node, const UA_QualifiedName key) {
    UA_BrowsePathResult bpr = UA_Server_browseSimplifiedBrowsePath(server, start_node, 1, &key);
    assert(bpr.statusCode == UA_STATUSCODE_GOOD && bpr.targetsSize > 0);
    *out_node_id = bpr.targets->targetId.nodeId;
    UA_BrowsePathResult_deleteMembers(&bpr);
}

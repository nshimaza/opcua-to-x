#ifndef UTIL_H
#define UTIL_H

void show_ua_string(char* out_str, size_t out_str_len, const UA_String src);
int show_node_id(char* out_str, size_t out_str_len, const UA_NodeId id);
void find_node_id(UA_Server *server, UA_NodeId* out_node_id, const UA_NodeId start_node, const UA_QualifiedName key);
#endif

#include <string.h>
#include <stdlib.h>
#include "chunk_node.h"
#include "chunk.h"

chunk_node_t* chunk_node_make(chunk_t chunk) {
    chunk_node_t* node = malloc(sizeof(chunk_node_t));
    memset(node, 0, sizeof(chunk_node_t));
    //memcpy(&node->chunk, &chunk, sizeof(chunk_t));
    return node;
}

chunk_node_t* chunk_node_build(uint8_t* start) {
    chunk_t chunk = chunk_decode(start);

    chunk_node_t* parent = chunk_node_make(chunk);

    if (chunk.type == CHUNK_TYPE_SET) {
        uint64_t remaining = chunk.total_length;
        uint8_t* data = chunk.data;
        while (remaining) {
            ast_node_t* child = ast_build(data);
            ast_node_append(parent, child);
            data = data + child->chunk.total_length;
            remaining = remaining - child->chunk.total_length;
        }
    }

    return parent;
}

#include <string.h>
#include <stdlib.h>
#include "chunk_node.h"
#include "chunk.h"

chunk_node_t* chunk_node_make() {
    chunk_node_t* node = malloc(sizeof(chunk_node_t));
    memset(node, 0, sizeof(chunk_node_t));
    return node;
}

void chunk_node_init(chunk_node_t* node, chunk_t chunk) {
    node->type = chunk.type;
    node->data_length = chunk.data_length;
}

chunk_t chunk_node_construct(uint8_t* start, chunk_node_t* node) {
    chunk_t chunk = chunk_decode(start);
    chunk_node_init(node, chunk);

    if (chunk.type == CHUNK_TYPE_SET) {
        node->nr_children = chunk_set_nr_items(chunk);
        node->children = malloc(sizeof(chunk_node_t) * node->nr_children);
        memset(node->children, 0, sizeof(chunk_node_t) * node->nr_children);
        uint64_t remaining = chunk.total_length;
        uint8_t* data = chunk.data;
        while (remaining) {
            chunk_t child = chunk_node_construct(data, node->children);
            node->children++;
            data = data + child.total_length;
            remaining = remaining - child.total_length;
        }
    }

    return chunk;
}

chunk_node_t* chunk_node_build(uint8_t* start) {
    chunk_node_t* node = chunk_node_make();
    chunk_node_construct(start, node);
    return node;
}

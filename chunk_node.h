#ifndef H_CHUNK_NODE
#define H_CHUNK_NODE

#include <stdint.h>
#include "chunk.h"

typedef struct chunk_node chunk_node_t;

typedef struct chunk_node {
    chunk_type_t type;
    uint8_t selected;
    uint8_t bytes_per_type;
    uint8_t* address;
    uint8_t* data;
    uint64_t nr_children;
    chunk_node_t* children;
} chunk_node_t;

chunk_node_t* chunk_node_select(chunk_node_t* node, uint64_t* addr, uint64_t nr_addr);

void chunk_node_destroy(chunk_node_t* node);

chunk_node_t* chunk_node_build(uint8_t* start);

#endif

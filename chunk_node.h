#ifndef H_AST
#define H_AST

#include <stdint.h>
#include "chunk.h"

typedef struct chunk_node chunk_node_t;

typedef struct chunk_node {
    chunk_type_t type;
    uint64_t nr_children;
    chunk_node_t* children;
    uint8_t bytes_per_type;
    uint8_t selected;
    uint8_t* address;
    uint8_t* data;
} chunk_node_t;

chunk_node_t* chunk_node_select(chunk_node_t* node, uint64_t* addr, uint64_t nr_addr);

chunk_node_t* chunk_node_build(uint8_t* start);

#endif

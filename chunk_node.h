#ifndef H_AST
#define H_AST

#include <stdint.h>
#include "chunk.h"

typedef struct chunk_node chunk_node_t;

typedef struct chunk_node {
    chunk_type_t type;
    uint64_t data_length;
    uint64_t nr_children;
    chunk_node_t* children;
    uint8_t selected;
    uint8_t* start;
    uint64_t byte_offset;
} chunk_node_t;

chunk_node_t* chunk_node_build(uint8_t* start);

#endif

#ifndef H_AST
#define H_AST

#include <stdint.h>
#include "chunk.h"

typedef struct ast_node ast_node_t;

typedef struct ast_node {
    uint8_t type;
    chunk_t* chunk;
    ast_node_t* children;
} ast_node_t;

uint64_t ast_build(uint8_t* data);

#endif

#ifndef H_AST
#define H_AST

#include <stdint.h>
#include "chunk.h"

typedef enum ast_node_type {
    AST_NODE_UNDEF,
    AST_NODE_SYMBOL,
    AST_NODE_EXPRESSION,
    AST_NODE_REFERENCE,
    AST_NODE_UTF8
} ast_node_type_t;

typedef struct ast_node ast_node_t;

typedef struct ast_node {
    ast_node_type_t type;
    chunk_t chunk;
    uint8_t nr_children;
    ast_node_t* children[255];
} ast_node_t;

ast_node_t* ast_build(uint8_t* start);

const char* ast_type_name(ast_node_type_t type);

#endif

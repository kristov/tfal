#include "ast.h"
#include "chunk.h"
#include <string.h>
#include <stdlib.h>

static const char* name_per_type[] = {
    "[undef]",
    "sym",
    "exp",
    "ref",
    "txt"
};

const char* ast_type_name(ast_node_type_t type) {
    return name_per_type[type];
}

void ast_set_type(ast_node_t* node) {
    if (node->nr_children > 0) {
        switch (node->children[0]->chunk.type) {
            case CHUNK_TYPE_UINT8:
                node->type = AST_NODE_SYMBOL;
                break;
            case CHUNK_TYPE_CHUNK:
                node->type = AST_NODE_EXPRESSION;
                break;
            case CHUNK_TYPE_UINT32:
                node->type = AST_NODE_REFERENCE;
                break;
            case CHUNK_TYPE_UTF8:
                node->type = AST_NODE_UTF8;
                break;
            default:
                node->type = AST_NODE_UNDEF;
        }
    }
}

void ast_node_append(ast_node_t* parent, ast_node_t* child) {
    parent->children[parent->nr_children] = child;
    parent->nr_children++;
}

ast_node_t* ast_node_make(chunk_t chunk) {
    ast_node_t* node = malloc(sizeof(ast_node_t));
    memset(node, 0, sizeof(ast_node_t));
    memcpy(&node->chunk, &chunk, sizeof(chunk_t));
    return node;
}

ast_node_t* ast_build(uint8_t* start) {
    chunk_t chunk = chunk_decode(start);

    ast_node_t* parent = ast_node_make(chunk);

    if (chunk.type == CHUNK_TYPE_CHUNK) {
        uint64_t remaining = chunk.length;
        uint8_t* data = chunk.data;
        while (remaining) {
            ast_node_t* child = ast_build(data);
            ast_node_append(parent, child);
            uint64_t child_total = chunk_total_length(child->chunk);
            data = data + child_total;
            remaining = remaining - child_total;
        }
    }
    ast_set_type(parent);

    return parent;
}

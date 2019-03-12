#ifndef H_CHUNK_NODE
#define H_CHUNK_NODE

#include <stdint.h>
#include "chunk.h"

#define NODE_FLAG_FOCUS 0x00
#define NODE_FLAG_REALISED 0x01
#define FLAG_SELECTED(n, f)  ((n->flags >> f) & 1)
#define FLAG_SELECT(n, f)    (n->flags |= (1 << f))
#define FLAG_UNSELECT(n, f)  (n->flags &= ~(1 << f))

typedef struct chunk_node_change {
    uint64_t insert_at;
    uint8_t* data;
    uint64_t nr_children;
} chunk_node_change_t;

typedef struct chunk_node chunk_node_t;

typedef struct chunk_node {
    chunk_type_t type;
    uint8_t flags;
    uint8_t bytes_per_type;
    uint8_t* address;
    uint8_t* data;
    uint64_t nr_children;
    chunk_node_t* children;
    chunk_node_change_t* change;
} chunk_node_t;

chunk_node_t* chunk_node_select(chunk_node_t* node, uint64_t* addr, uint64_t nr_addr);

chunk_node_t* chunk_node_set_insert(chunk_node_t* node, uint64_t location);

void chunk_node_destroy(chunk_node_t* node);

chunk_node_t* chunk_node_build(uint8_t* start);

#endif

#ifndef H_CHUNK_NODE
#define H_CHUNK_NODE

#include <stdint.h>
#include "chunk.h"

#define NODE_FLAG_FOCUS 0x00
#define NODE_FLAG_REALISED 0x01

#define BIT_TEST(a, f)   ((a >> f) & 1)
#define BIT_SET(a, f)    (a |= (1 << f))
#define BIT_UNSET(a, f)  (a &= ~(1 << f))

#define FLAG_SELECTED(n, f)  BIT_TEST(n->flags, f)
#define FLAG_SELECT(n, f)    BIT_SET(n->flags, f)
#define FLAG_UNSELECT(n, f)  BIT_UNSET(n->flags, f)

typedef struct chunk_node chunk_node_t;

typedef struct chunk_node {
    chunk_type_t type;
    uint8_t flags;
    uint64_t data_length;
    uint8_t* address;
    uint8_t* data;
    uint64_t nr_children;
    chunk_node_t* children;
} chunk_node_t;

uint64_t chunk_node_size(chunk_node_t* node);

chunk_node_t* chunk_node_select(chunk_node_t* node, uint64_t* addr, uint64_t nr_addr);

uint8_t* chunk_node_data_insert(chunk_node_t* node, uint64_t location, uint8_t* data, uint64_t nr_bytes);

chunk_node_t* chunk_node_set_insert(chunk_node_t* node, uint64_t location);

void chunk_node_load_data_unrealise(chunk_node_t* node);

void chunk_node_destroy(chunk_node_t* node);

chunk_node_t* chunk_node_build(uint8_t* start);

#endif

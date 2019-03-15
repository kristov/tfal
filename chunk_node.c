#include <string.h>
#include <stdlib.h>
#include "chunk_node.h"
#include "chunk.h"
#include "utf8.h"

chunk_node_t* chunk_node_make() {
    chunk_node_t* node = malloc(sizeof(chunk_node_t));
    memset(node, 0, sizeof(chunk_node_t));
    return node;
}

void chunk_node_init(chunk_node_t* node, chunk_t chunk, uint8_t* start) {
    node->type = chunk.type;
    node->address = chunk.address;
    node->data = chunk.data;
    node->data_length = chunk.data_length;
    FLAG_SELECT(node, NODE_FLAG_REALISED);
    switch (chunk.type) {
        case CHUNK_TYPE_SET:
            node->nr_children = chunk_set_nr_items(chunk);
            break;
        case CHUNK_TYPE_UTF8:
            node->nr_children = u8_strlen((char*)chunk.data);
            break;
        default:
            node->nr_children = chunk.data_length / chunk_bytes_per_type(node->type);
            break;
    }
}

chunk_node_t* chunk_node_select(chunk_node_t* node, uint64_t* addr, uint64_t nr_addr) {
    if (node->type != CHUNK_TYPE_SET) {
        return NULL;
    }
    if (addr[0] >= node->nr_children) {
        return NULL;
    }
    chunk_node_t* child = &node->children[addr[0]];
    nr_addr--;
    if (nr_addr == 0) {
        return child;
    }
    return chunk_node_select(child, &addr[1], nr_addr);
}

chunk_node_t* chunk_node_set_insert(chunk_node_t* node, uint64_t location) {
    if (node->type != CHUNK_TYPE_SET) {
        return NULL;
    }
    if (location > node->nr_children) {
        return NULL;
    }
    chunk_node_t* children_new = malloc(sizeof(chunk_node_t) * (node->nr_children + 1));

    if (location == node->nr_children) {
        memcpy(children_new, node->children, sizeof(chunk_node_t) * node->nr_children);
    }
    else {
        if (location > 0) {
            memcpy(&children_new[0], &node->children[0], sizeof(chunk_node_t) * location);
        }
        memcpy(&children_new[location + 1], &node->children[location], sizeof(chunk_node_t) * (node->nr_children - location));
    }
    memset(&children_new[location], 0, sizeof(chunk_node_t));
    free(node->children);
    node->children = children_new;
    node->nr_children++;
    return &children_new[location];
}

void chunk_node_load_data_unrealise(chunk_node_t* node) {
    uint64_t bytes_to_load = chunk_bytes_per_type(node->type) * node->nr_children;
    uint8_t* loaded_data = malloc(bytes_to_load);
    memcpy(loaded_data, node->data, bytes_to_load);
    node->data = loaded_data;
    FLAG_UNSELECT(node, NODE_FLAG_REALISED);
}

void chunk_node_destroy_tree(chunk_node_t* node) {
    if (node->type == CHUNK_TYPE_SET) {
        for (uint64_t i = 0; i < node->nr_children; i++) {
            chunk_node_t* child = &node->children[i];
            chunk_node_destroy_tree(child);
        }
        free(node->children);
        node->children = NULL;
        return;
    }
    if (!FLAG_SELECTED(node, NODE_FLAG_REALISED)) {
        free(node->data);
    }
}

void chunk_node_destroy(chunk_node_t* node) {
    chunk_node_destroy_tree(node);
    free(node);
}

chunk_t chunk_node_construct(uint8_t* start, chunk_node_t* node) {
    chunk_t chunk = chunk_decode(start);
    chunk_node_init(node, chunk, start);

    if (chunk.type == CHUNK_TYPE_SET) {
        node->nr_children = chunk_set_nr_items(chunk);
        size_t size = node->nr_children * sizeof(chunk_node_t);
        node->children = (chunk_node_t*)malloc(size);
        memset(node->children, 0, size);
        uint64_t remaining = chunk.data_length;
        uint8_t* data = chunk.data;
        uint64_t i = 0;
        while (remaining) {
            chunk_t child = chunk_node_construct(data, &node->children[i]);
            i++;
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

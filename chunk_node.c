#include <string.h>
#include <stdlib.h>
#include "chunk_node.h"
#include "chunk.h"
#include "utf8.h"

uint64_t chunk_node_size(chunk_node_t* node) {
    uint64_t size = 0;
    if (node->type == CHUNK_TYPE_SET) {
        size = 9;
        for (uint64_t i = 0; i < node->nr_children; i++) {
            chunk_node_t* child = &node->children[i];
            size += chunk_node_size(child);
        }
        return size;
    }
    return chunk_nr_length_bytes(node->data_length) + 1 + node->data_length;
}

chunk_node_t* chunk_node_make() {
    chunk_node_t* node = malloc(sizeof(chunk_node_t));
    memset(node, 0, sizeof(chunk_node_t));
    return node;
}

void chunk_node_init(chunk_node_t* node, chunk_t chunk, uint8_t* start) {
    node->type = chunk.type;
    node->address = chunk.address;
    node->data_length = chunk.data_length;
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
    if (chunk.type != CHUNK_TYPE_SET) {
        node->data = malloc(sizeof(uint8_t) * chunk.data_length);
        memcpy(node->data, chunk.data, chunk.data_length);
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

uint8_t* chunk_node_data_insert(chunk_node_t* node, uint64_t location, uint8_t* data, uint64_t nr_bytes) {
    if (node->type == CHUNK_TYPE_SET) {
        return NULL;
    }
    if (location > node->data_length) {
        return NULL;
    }
    uint8_t* data_new = malloc(sizeof(uint8_t) * (node->data_length + nr_bytes));

    if (location == node->data_length) {
        memcpy(&data_new[0], node->data, sizeof(uint8_t) * node->data_length);
    }
    else {
        if (location > 0) {
            memcpy(&data_new[0], &node->data[0], sizeof(uint8_t) * location);
        }
        memcpy(&data_new[location + 1], &node->data[location], sizeof(uint8_t) * (node->data_length - location));
    }
    return node->data;
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
        memcpy(&children_new[0], node->children, sizeof(chunk_node_t) * node->nr_children);
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
    node->data_length = chunk_nr_length_bytes(0) + 1;
    return &children_new[location];
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
    if (node->data != NULL) {
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

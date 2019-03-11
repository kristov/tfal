#include "../chunk_node.h"
#include "test_harness.h"
#include <stdio.h>

uint8_t TEST_STRUCTURE[] = {
    0x8d,
    0x1b,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x11,
    0x01,
    0x09,
    0x8d,
    0x09,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x11,
    0x01,
    0x09,
    0x11,
    0x01,
    0x08,
    0x12,
    0x01,
    0x07,
    0x11,
    0x01,
    0x08,
    0x11,
    0x01,
    0x07
};

void test_chunk_node_build(test_harness_t* test) {
    chunk_node_t* root = chunk_node_build(TEST_STRUCTURE);
    chunk_node_t* node = root;

    is_equal_uint64(test, node->nr_children, 4, "test_chunk_node_build(): [] nr_children");
    is_equal_uint8(test, node->children[0].type, 1, "test_chunk_node_build(): [0] type");
    is_equal_uint8(test, node->children[0].nr_children, 1, "test_chunk_node_build(): [0] nr_children");

    is_equal_uint8(test, node->children[1].type, 13, "test_chunk_node_build(): [1] type");
    is_equal_uint8(test, node->children[1].nr_children, 3, "test_chunk_node_build(): [1] nr_children");

    node = &node->children[1];
    is_equal_uint64(test, node->nr_children, 3, "test_chunk_node_build(): [1] nr_children");
    is_equal_uint8(test, node->children[0].type, 1, "test_chunk_node_build(): [1:0] type");
    is_equal_uint8(test, node->children[0].nr_children, 1, "test_chunk_node_build(): [1:0] nr_children");

    is_equal_uint8(test, node->children[1].type, 1, "test_chunk_node_build(): [1:2] type");
    is_equal_uint8(test, node->children[1].nr_children, 1, "test_chunk_node_build(): [1:2] nr_children");

    is_equal_uint8(test, node->children[2].type, 2, "test_chunk_node_build(): [1:2] type");
    is_equal_uint8(test, node->children[2].nr_children, 1, "test_chunk_node_build(): [1:2] nr_children");

    node = root;
    is_equal_uint8(test, node->children[2].type, 1, "test_chunk_node_build(): [2] type");
    is_equal_uint8(test, node->children[2].nr_children, 1, "test_chunk_node_build(): [2] nr_children");

    is_equal_uint8(test, node->children[3].type, 1, "test_chunk_node_build(): [3] type");
    is_equal_uint8(test, node->children[3].nr_children, 1, "test_chunk_node_build(): [3] nr_children");

    chunk_node_destroy(root);
}

void test_chunk_node_add(test_harness_t* test) {
    chunk_node_t* root = chunk_node_build(TEST_STRUCTURE);
    chunk_node_t* node = root;

}

int main(int argc, char** argv) {
    test_harness_t test;
    test_harness_init(&test);
    test.verbose = 1;

    test_chunk_node_build(&test);
    test_chunk_node_add(&test);

    test_harness_report(&test);
    return 0;
}

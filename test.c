#include "ast.h"
#include <stdio.h>

void spaces(uint16_t depth) {
    for (uint16_t i = 0; i < depth; i++) {
        fprintf(stderr, "  ");
    }
}

void walk(ast_node_t* node, uint16_t depth) {
    spaces(depth); fprintf(stderr, "type: %s\n", ast_type_name(node->type));
    spaces(depth); fprintf(stderr, "nr_children: %d\n", node->nr_children);
    for (uint8_t i = 0; i < node->nr_children; i++) {
        walk(node->children[i], depth + 1);
    }
}

int main(int argc, char** argv) {
    uint8_t data[] = {
        0x1c, // 1 length byte, data type 12
        0x14, // 20 bytes long
        0x15, //   1 length byte, data type 5
        0x01, //   1 bytes long
        0x09, //   data (9)
        0x1c, //   1 length byte, data type 12
        0x09, //   9 bytes long
        0x15, //     1 length byte, data type 5
        0x01, //     1 bytes long
        0x09, //     data (9)
        0x11, //     1 length byte, data type 1
        0x01, //     1 bytes long
        0x08, //     data (8)
        0x12, //     1 length byte, data type 2
        0x01, //     1 bytes long
        0x07, //     data (7)
        0x11, //   1 length byte, data type 1
        0x01, //   1 bytes long
        0x08, //   data (8)
        0x12, //   1 length byte, data type 2
        0x01, //   1 bytes long
        0x07  //   data (7)
    };

    ast_node_t* root = ast_build(&data[0]);
    walk(root, 0);
    return 0;
}

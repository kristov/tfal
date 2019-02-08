#include "ast.h"
#include <stdio.h>

int main(int argc, char** argv) {
    uint8_t data[] = {
        0x1c, // 1 length byte, data type 2
        0x14, // 20 bytes long
        0x10, //   1 length byte, data type 0
        0x01, //   1 bytes long
        0x09, //   data (9)
        0x1c, //   1 length byte, data type 2
        0x09, //   9 bytes long
        0x10, //     1 length byte, data type 0
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

    uint64_t counter = ast_build(&data[0]);
    fprintf(stderr, "nr_objects: %lu\n", counter);
    return 0;
}

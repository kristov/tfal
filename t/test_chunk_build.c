#include "../chunk.h"
#include "test_harness.h"
#include <stdio.h>
#include <stdlib.h>

uint8_t TEST_STRUCTURE[] = {
    0x8d, // 8 length bytes, data type 12
    0x1b, // 27 bytes long
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x15, //   1 length byte, data type 5
    0x01, //   1 bytes long
    0x09, //   data (9)
    0x8d, //   1 length byte, data type 12
    0x09, //   9 bytes long
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x15, //     1 length byte, data type 5
    0x01, //     1 bytes long
    0x09, //     data (9)
    0x11, //     1 length byte, data type 1
    0x01, //     1 bytes long
    0x08, //     data (8)
    0x12, //     1 length byte, data type 2
    0x01, //     1 bytes long
    0x07, //     data (7)
    0x13, //   1 length byte, data type 3
    0x01, //   1 bytes long
    0x08, //   data (8)
    0x12, //   1 length byte, data type 2
    0x01, //   1 bytes long
    0x07  //   data (7)
};

void test_build_replica(test_harness_t* test) {
    uint8_t* data = malloc(sizeof(uint8_t) * 36);
    uint64_t addr1 = 0;
    uint64_t addr2 = 0;
    chunk_t work;

    chunk_write_header(data, CHUNK_TYPE_SET, 27);
    work = chunk_decode(data);

    addr1 = chunk_set_item_byte_offset(work, 0);
    is_equal_uint64(test, addr1, 9, "test_build_replica(): first level set offset 0 correct");
    chunk_write_header(&data[addr1], CHUNK_TYPE_UINT32, 1);

    addr1 = chunk_set_item_byte_offset(work, 1);
    is_equal_uint64(test, addr1, 12, "test_build_replica(): first level set offset 1 correct");
    chunk_write_header(&data[addr1], CHUNK_TYPE_SET, 9);
    work = chunk_decode(&data[addr1]);
    *work.data = 9;

    work = chunk_decode(&data[addr1]);

    addr2 = chunk_set_item_byte_offset(work, 0);
    chunk_write_header(&data[addr1 + addr2], CHUNK_TYPE_UINT32, 1);

    addr2 = chunk_set_item_byte_offset(work, 1);
    chunk_write_header(&data[addr1 + addr2], CHUNK_TYPE_UINT8, 1);

    addr2 = chunk_set_item_byte_offset(work, 2);
    chunk_write_header(&data[addr1 + addr2], CHUNK_TYPE_INT8, 1);

    work = chunk_decode(data);

    addr1 = chunk_set_item_byte_offset(work, 2);
    chunk_write_header(&data[addr1], CHUNK_TYPE_UINT16, 1);

    addr1 = chunk_set_item_byte_offset(work, 3);
    chunk_write_header(&data[addr1], CHUNK_TYPE_INT8, 1);

    for (uint8_t i = 0; i < 36; i++) {
        is_equal_uint8(test, data[i], TEST_STRUCTURE[i], "test_build_replica(): iter");
    }
}

int main(int argc, char** argv) {
    test_harness_t test;
    test_harness_init(&test);
    test.verbose = 1;

    test_build_replica(&test);

    return 0;
}

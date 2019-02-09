#include "../chunk.h"
#include "test_harness.h"
#include <stdio.h>

void test_encode_simple(test_harness_t* test) {
    chunk_t chunk;
    uint8_t data[2];

    chunk.length = 4;
    chunk.type = CHUNK_TYPE_UINT8;
    chunk.nr_length_bytes = chunk_nr_length_bytes(chunk.length);

    chunk_make(&data[0], chunk);

    is_equal_uint8(test, data[0], 0x11, "header correct");
    is_equal_uint8(test, data[1], 0x04, "length byte correct");
}

void test_encode_long(test_harness_t* test) {
    chunk_t chunk;
    uint8_t data[3];

    chunk.length = 256;
    chunk.type = CHUNK_TYPE_UINT8;
    chunk.nr_length_bytes = chunk_nr_length_bytes(chunk.length);

    chunk_make(&data[0], chunk);

    is_equal_uint8(test, data[0], 0x21, "header correct");
    is_equal_uint8(test, data[1], 0x00, "first length byte correct");
    is_equal_uint8(test, data[2], 0x01, "second length byte correct");
}

void test_encode_longer(test_harness_t* test) {
    chunk_t chunk;
    uint8_t data[6];

    chunk.length = 16909060;
    chunk.type = CHUNK_TYPE_UINT8;
    chunk.nr_length_bytes = chunk_nr_length_bytes(chunk.length);

    chunk_make(&data[0], chunk);

    // 16909060 == 00000001 00000010 00000011 00000100
    is_equal_uint8(test, data[0], 0x41, "header correct");
    is_equal_uint8(test, data[1], 0x04, "first length byte correct");
    is_equal_uint8(test, data[2], 0x03, "second length byte correct");
    is_equal_uint8(test, data[3], 0x02, "third length byte correct");
    is_equal_uint8(test, data[4], 0x01, "forth length byte correct");
}

void test_decode_longer(test_harness_t* test) {
    uint8_t data[] = {
        0x41,
        0x04,
        0x03,
        0x02,
        0x01
    };
    chunk_t chunk = chunk_decode(data);

    is_equal_uint64(test, chunk.length, 16909060, "chunk is 16909060 bytes long");
    is_equal_uint8(test, chunk.type, CHUNK_TYPE_UINT8, "chunk is type CHUNK_TYPE_UINT8");
}

void test_chunk_nr_length_bytes(test_harness_t* test) {
    is_equal_uint8(test, chunk_nr_length_bytes(255), 1, "computed length bytes [1]");
    is_equal_uint8(test, chunk_nr_length_bytes(256), 2, "computed length bytes [2]");
    is_equal_uint8(test, chunk_nr_length_bytes(65535), 2, "computed length bytes [2]");
    is_equal_uint8(test, chunk_nr_length_bytes(65536), 3, "computed length bytes [3]");
    is_equal_uint8(test, chunk_nr_length_bytes(16777215), 3, "computed length bytes [3]");
    is_equal_uint8(test, chunk_nr_length_bytes(16777216), 4, "computed length bytes [4]");
    is_equal_uint8(test, chunk_nr_length_bytes(4294967295), 4, "computed length bytes [4]");
    is_equal_uint8(test, chunk_nr_length_bytes(4294967296), 5, "computed length bytes [4]");
}

void test_decode_simple(test_harness_t* test) {
    uint8_t data[] = {
        0x11,
        0x01,
        0x09
    };
    chunk_t chunk = chunk_decode(data);

    is_equal_uint64(test, chunk.length, 1, "chunk is 1 bytes long");
    is_equal_uint8(test, chunk.type, CHUNK_TYPE_UINT8, "chunk root is type CHUNK_TYPE_UINT8");
}

void test_decode_complex(test_harness_t* test) {
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
    chunk_t chunk = chunk_decode(data);

    is_equal_uint32(test, chunk.length, 20, "chunk is 20 bytes long");
    is_equal_uint8(test, chunk.type, CHUNK_TYPE_CHUNK, "chunk root is type CHUNK_TYPE_CHUNK");
}

int main(int argc, char** argv) {
    test_harness_t* test = test_harness_create();
    test->verbose = 1;

    test_decode_simple(test);
    test_decode_longer(test);
    test_decode_complex(test);

    test_chunk_nr_length_bytes(test);

    test_encode_simple(test);
    test_encode_long(test);
    test_encode_longer(test);

    test_harness_report(test);
    return 0;
}

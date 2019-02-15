#include "../chunk.h"
#include "test_harness.h"
#include <stdio.h>

void test_encode_simple(test_harness_t* test) {
    chunk_t chunk;
    uint8_t data[2];

    chunk.total_length = 4;
    chunk.type = CHUNK_TYPE_UINT8;
    chunk.nr_length_bytes = chunk_nr_length_bytes(chunk.total_length);

    chunk_make(&data[0], chunk);

    is_equal_uint8(test, data[0], 0x11, "test_encode_simple(): header correct");
    is_equal_uint8(test, data[1], 0x04, "test_encode_simple(): length byte correct");
}

void test_encode_long(test_harness_t* test) {
    chunk_t chunk;
    uint8_t data[3];

    chunk.total_length = 256;
    chunk.type = CHUNK_TYPE_UINT8;
    chunk.nr_length_bytes = chunk_nr_length_bytes(chunk.total_length);

    chunk_make(&data[0], chunk);

    is_equal_uint8(test, data[0], 0x21, "test_encode_long(): header correct");
    is_equal_uint8(test, data[1], 0x00, "test_encode_long(): first length byte correct");
    is_equal_uint8(test, data[2], 0x01, "test_encode_long(): second length byte correct");
}

void test_encode_longer(test_harness_t* test) {
    chunk_t chunk;
    uint8_t data[6];

    chunk.total_length = 16909060;
    chunk.type = CHUNK_TYPE_UINT8;
    chunk.nr_length_bytes = chunk_nr_length_bytes(chunk.total_length);

    chunk_make(&data[0], chunk);

    // 16909060 == 00000001 00000010 00000011 00000100
    is_equal_uint8(test, data[0], 0x41, "test_encode_longer(): header correct");
    is_equal_uint8(test, data[1], 0x04, "test_encode_longer(): first length byte correct");
    is_equal_uint8(test, data[2], 0x03, "test_encode_longer(): second length byte correct");
    is_equal_uint8(test, data[3], 0x02, "test_encode_longer(): third length byte correct");
    is_equal_uint8(test, data[4], 0x01, "test_encode_longer(): forth length byte correct");
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

    is_equal_uint64(test, chunk.data_length, 16909060, "test_decode_longer(): chunk is 16909060 bytes long");
    is_equal_uint8(test, chunk.type, CHUNK_TYPE_UINT8, "test_decode_longer(): chunk is type CHUNK_TYPE_UINT8");
}

void test_chunk_nr_length_bytes(test_harness_t* test) {
    is_equal_uint8(test, chunk_nr_length_bytes(255), 1, "test_chunk_nr_length_bytes(): computed length bytes [1]");
    is_equal_uint8(test, chunk_nr_length_bytes(256), 2, "test_chunk_nr_length_bytes(): computed length bytes [2]");
    is_equal_uint8(test, chunk_nr_length_bytes(65535), 2, "test_chunk_nr_length_bytes(): computed length bytes [2]");
    is_equal_uint8(test, chunk_nr_length_bytes(65536), 3, "test_chunk_nr_length_bytes(): computed length bytes [3]");
    is_equal_uint8(test, chunk_nr_length_bytes(16777215), 3, "test_chunk_nr_length_bytes(): computed length bytes [3]");
    is_equal_uint8(test, chunk_nr_length_bytes(16777216), 4, "test_chunk_nr_length_bytes(): computed length bytes [4]");
    is_equal_uint8(test, chunk_nr_length_bytes(4294967295), 4, "test_chunk_nr_length_bytes(): computed length bytes [4]");
    is_equal_uint8(test, chunk_nr_length_bytes(4294967296), 5, "test_chunk_nr_length_bytes(): computed length bytes [4]");
}

void test_decode_simple(test_harness_t* test) {
    uint8_t data[] = {
        0x11,
        0x01,
        0x09
    };
    chunk_t chunk = chunk_decode(data);

    is_equal_uint64(test, chunk.data_length, 1, "test_decode_simple(): chunk is 1 bytes long");
    is_equal_uint8(test, chunk.type, CHUNK_TYPE_UINT8, "test_decode_simple(): chunk is type CHUNK_TYPE_UINT8");
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
        0x13, //   1 length byte, data type 3
        0x01, //   1 bytes long
        0x08, //   data (8)
        0x12, //   1 length byte, data type 2
        0x01, //   1 bytes long
        0x07  //   data (7)
    };
    chunk_t chunk = chunk_decode(data);

    is_equal_uint64(test, chunk.data_length, 20, "test_decode_complex(): chunk is 20 bytes long");
    is_equal_uint8(test, chunk.type, CHUNK_TYPE_SET, "test_decode_complex(): chunk root is type CHUNK_TYPE_SET");

    chunk_t walk;
    uint8_t found = chunk_set_get_nth(chunk, &walk, 2);
    is_equal_uint8(test, found, 1, "test_decode_complex(): found the third element in set");
    is_equal_uint8(test, walk.type, CHUNK_TYPE_UINT16, "test_decode_complex(): third element in set is CHUNK_TYPE_UINT16");

    found = chunk_set_get_nth(chunk, &walk, 1);
    is_equal_uint8(test, found, 1, "test_decode_complex(): found the second element in set");
    is_equal_uint8(test, walk.type, CHUNK_TYPE_SET, "test_decode_complex(): second element in set is CHUNK_TYPE_SET");

    found = chunk_set_get_nth(walk, &walk, 2);
    is_equal_uint8(test, found, 1, "test_decode_complex(): found the third element in sub-set");
    is_equal_uint8(test, walk.type, CHUNK_TYPE_INT8, "test_decode_complex(): third element in sub-set is CHUNK_TYPE_INT8");
}

void test_chunk_get_offset_simple(test_harness_t* test) {
    uint8_t data[] = {
        0x1c,
        0x06,
        0x15,
        0x01,
        0x09,
        0x15,
        0x01,
        0x0a,
    };
    chunk_t chunk = chunk_decode(&data[0]);
    uint64_t offset = 0;

    uint32_t addrb[1] = {0};
    offset = chunk_byte_offset(chunk, &addrb[0], 1);
    is_equal_uint64(test, offset, 2, "test_chunk_get_offset_simple(): offset correct (at beginning)");

    uint32_t addra[1] = {1};
    offset = chunk_byte_offset(chunk, &addra[0], 1);
    is_equal_uint64(test, offset, 5, "test_chunk_get_offset_simple(): offset correct (in middle)");

    uint32_t addrc[1] = {2};
    offset = chunk_byte_offset(chunk, &addrc[0], 1);
    is_equal_uint64(test, offset, 8, "test_chunk_get_offset_simple(): offset correct (at end)");

    uint32_t addrd[1] = {3};
    offset = chunk_byte_offset(chunk, &addrd[0], 1);
    is_equal_uint64(test, offset, 0, "test_chunk_get_offset_simple(): offset zero (overflow)");
}

void test_chunk_get_offset(test_harness_t* test) {
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
        0x13, //   1 length byte, data type 3
        0x01, //   1 bytes long
        0x08, //   data (8)
        0x12, //   1 length byte, data type 2
        0x01, //   1 bytes long
        0x07  //   data (7)
    };
    chunk_t chunk = chunk_decode(&data[0]);
    uint64_t offset = 0;

    uint32_t addra[2] = {1, 1};
    offset = chunk_byte_offset(chunk, &addra[0], 2);
    is_equal_uint64(test, offset, 10, "test_chunk_get_offset(): offset correct (expected)");

    uint32_t addrb[2] = {1, 3};
    offset = chunk_byte_offset(chunk, &addrb[0], 2);
    is_equal_uint64(test, offset, 16, "test_chunk_get_offset(): offset correct (end of sub set)");

    uint32_t addrc[2] = {2, 2};
    offset = chunk_byte_offset(chunk, &addrc[0], 2);
    is_equal_uint64(test, offset, 0, "test_chunk_get_offset(): offset zero (first level not a set)");

    uint32_t addrd[2] = {1, 4};
    offset = chunk_byte_offset(chunk, &addrd[0], 2);
    is_equal_uint64(test, offset, 0, "test_chunk_get_offset(): offset zero (overflow second sub set)");
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

    test_chunk_get_offset_simple(test);
    test_chunk_get_offset(test);

    test_harness_report(test);
    return 0;
}

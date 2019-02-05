#include <stdint.h>
#include <stdio.h>

uint8_t bytes_per_type[] = {
    0x01, // uint8
    0x01, // int8
    0x02, // uint16
    0x02, // int16
    0x04, // uint32
    0x04, // int32
    0x08, // uint64
    0x08, // int64
    0x04, // float32
    0x08, // float64
    0x01, // utf8
    0x00, // chunk (length not defined)
};

const char* name_per_type[] = {
    "uint8",
    "int8",
    "uint16",
    "int16",
    "uint32",
    "int32",
    "uint64",
    "int64",
    "float32",
    "float64",
    "utf8",
    "[undef]",
    "chunk"
};

typedef struct chunk {
    uint8_t type;
    uint8_t nr_length_bytes;
    uint64_t length;
    uint8_t* data;
} chunk_t;

typedef void (*chunk_walk_callback_t)(chunk_t chunk, void* payload);

void chunk_walk_object_counter(chunk_t chunk, void* payload) {
    (*(uint64_t*)payload)++;
}

chunk_t chunk_walk_pointer(uint8_t* pointer, chunk_walk_callback_t callback, void* payload) {
    uint64_t index = 1;
    uint8_t head_byte = pointer[0];

    chunk_t chunk;
    chunk.type = (head_byte & 0x0f);
    chunk.nr_length_bytes = ((head_byte >> 0x04) & 0x0f);

    chunk.length = 0;
    for (uint8_t i = 0; i < chunk.nr_length_bytes; i++) {
        chunk.length |= (pointer[index] << (0x08 * i));
        index++;
    }
    chunk.data = &pointer[index];

    callback(chunk, payload);

    if (chunk.type == 0x0c) {
        uint64_t remaining = chunk.length;
        uint8_t* data = chunk.data;
        while (remaining) {
            chunk_t child = chunk_walk_pointer(data, callback, payload);
            uint64_t child_total = 1 + child.nr_length_bytes + child.length;
            data = data + child_total;
            remaining = remaining - child_total;
        }
    }

    return chunk;
}

int main(int argc, char** argv) {

    //0x14
    //0x95
    //0x82
    //0x4f

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
    uint64_t counter;
    chunk_walk_pointer(&data[0], chunk_walk_object_counter, &counter);

    fprintf(stderr, "nr_objects: %lu\n", counter);
    return 0;
}

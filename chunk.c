#include "chunk.h"

uint8_t bytes_per_type[] = {
    0x00, // undef
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
    "[undef]",
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
    "chunk"
};

uint8_t chunk_bytes_per_type(chunk_t chunk) {
    return bytes_per_type[chunk.type];
}

chunk_t chunk_walk_pointer(uint8_t* pointer, chunk_callback_t callback, void* payload) {
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

    if (chunk.type == CHUNK_TYPE_CHUNK) {
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

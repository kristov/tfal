#include "chunk.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>

#include <stdio.h>

static uint8_t bytes_per_type[] = {
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
    0x00, // set (unknown)
};

static const char* name_per_type[] = {
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
    "set"
};

const char* chunk_type_name(chunk_type_t type) {
    return name_per_type[type];
}

uint8_t chunk_bytes_per_type(chunk_t chunk) {
    return bytes_per_type[chunk.type];
}

uint8_t chunk_nr_length_bytes(uint64_t length) {
    uint8_t ans = 0;
    while (length >>= 1) ans++;
    return (uint8_t)(ans / 8) + 1;
}

void chunk_make(uint8_t* start, chunk_t chunk) {
    uint8_t head = 0;
    head = (head & 0xf0) | (chunk.type & 0xf);
    head = (head & 0x0f) | ((chunk.nr_length_bytes & 0xf) << 4);
    *start = head;
    start++;
    for (uint8_t i = 0; i < chunk.nr_length_bytes; i++) {
        *start = 0x00;
        *start = (chunk.total_length >> (0x08 * i));
        start++;
    }
}

chunk_t chunk_decode(uint8_t* start) {
    uint64_t index = 1;
    uint8_t head_byte = start[0];

    chunk_t chunk;
    chunk.type = (head_byte & 0x0f);
    chunk.nr_length_bytes = ((head_byte >> 0x04) & 0x0f);

    chunk.data_length = 0;
    for (uint8_t i = 0; i < chunk.nr_length_bytes; i++) {
        chunk.data_length |= (start[index] << (0x08 * i));
        index++;
    }

    chunk.total_length = 1 + chunk.nr_length_bytes + chunk.data_length;
    chunk.data = &start[index];
    return chunk;
}

uint8_t chunk_set_get_nth(chunk_t chunk, chunk_t* dest, uint64_t nth) {
    if (chunk.type != CHUNK_TYPE_SET) {
        return 0;
    }
    uint64_t remaining = chunk.data_length;
    uint8_t* data = chunk.data;
    uint64_t count = 0;
    while (remaining) {
        chunk_t child = chunk_decode(data);
        if (count == nth) {
            memcpy(dest, &child, sizeof(chunk_t));
            return 1;
        }
        data = data + child.total_length;
        remaining = remaining - child.total_length;
        count++;
    }
    return 0;
}

uint64_t chunk_byte_offset(chunk_t chunk, uint32_t* idx, uint32_t nr_idx) {
    if (chunk.type != CHUNK_TYPE_SET) {
        return 0;
    }
    uint64_t remaining = chunk.data_length;
    uint8_t* data = chunk.data;
    uint64_t count = 0;
    uint64_t offset = 1 + chunk.nr_length_bytes;
    while (remaining) {
        chunk_t child = chunk_decode(data);

        if (count == idx[0]) {
            idx = &idx[1];
            nr_idx--;
            if (nr_idx == 0) {
                return offset;
            }
            if (child.type == CHUNK_TYPE_SET) {
                uint64_t child_offset = chunk_byte_offset(child, idx, nr_idx);
                if (child_offset == 0) {
                    return 0;
                }
                return offset + child_offset;
            }
        }

        offset += child.total_length;
        data = data + child.total_length;
        remaining = remaining - child.total_length;
        count++;
    }
    if (count == idx[0]) {
        nr_idx--;
        if (nr_idx == 0) {
            return offset;
        }
    }

    return 0;
}

uint64_t chunk_set_nr_items(chunk_t chunk) {
    if (chunk.type != CHUNK_TYPE_SET) {
        return 0;
    }
    uint64_t remaining = chunk.data_length;
    uint8_t* data = chunk.data;
    uint64_t count = 0;
    while (remaining) {
        chunk_t child = chunk_decode(data);
        data = data + child.total_length;
        remaining = remaining - child.total_length;
        count++;
    }
    return count;
}

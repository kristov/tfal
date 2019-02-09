#ifndef H_CHUNK
#define H_CHUNK

#include <stdint.h>

typedef enum chunk_type {
    CHUNK_TYPE_UNDEF,
    CHUNK_TYPE_UINT8,
    CHUNK_TYPE_INT8,
    CHUNK_TYPE_UINT16,
    CHUNK_TYPE_INT16,
    CHUNK_TYPE_UINT32,
    CHUNK_TYPE_INT32,
    CHUNK_TYPE_UINT64,
    CHUNK_TYPE_INT64,
    CHUNK_TYPE_FLOAT32,
    CHUNK_TYPE_FLOAT64,
    CHUNK_TYPE_UTF8,
    CHUNK_TYPE_CHUNK
} chunk_type_t;

typedef struct chunk {
    chunk_type_t type;
    uint8_t nr_length_bytes;
    uint64_t length;
    uint8_t* data;
} chunk_t;

uint8_t chunk_bytes_per_type(chunk_t chunk);

const char* chunk_type_name(chunk_type_t type);

uint8_t chunk_nr_length_bytes(uint64_t length);

void chunk_make(uint8_t* start, chunk_t chunk);

chunk_t chunk_decode(uint8_t* start);

uint64_t chunk_total_length(chunk_t chunk);

#endif

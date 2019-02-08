#include "ast.h"
#include "chunk.h"

static void object_counter(chunk_t chunk, void* payload) {
    (*(uint64_t*)payload)++;
}

uint64_t ast_build(uint8_t* data) {
    uint64_t counter = 0;
    chunk_walk_pointer(data, object_counter, &counter);
    return counter;
}

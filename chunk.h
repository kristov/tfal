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
    CHUNK_TYPE_SET
} chunk_type_t;

typedef struct chunk {
    chunk_type_t type;
    uint8_t nr_length_bytes;
    uint64_t length;
    uint8_t* data;
} chunk_t;

/**
 * @brief Number of bytes for a given type
 *
 * Returns the number of bytes for the type of the chunk.
 *
 * @param chunk A chunk
 * @return Number of bytes for that chunk type
 */
uint8_t chunk_bytes_per_type(chunk_t chunk);

/**
 * @brief Number of bytes for a given type
 *
 * Returns the number of bytes for the type of the chunk.
 *
 * @param chunk A chunk
 * @return Number of bytes for that chunk type
 */
const char* chunk_type_name(chunk_type_t type);

/**
 * @brief Number of length bytes for a given length
 *
 * Returns the number of length bytes that are needed to store the chunk data
 * length.
 *
 * @param length The length of the data in the chunk
 * @return The number of length bytes needed to store that
 */
uint8_t chunk_nr_length_bytes(uint64_t length);

/**
 * @brief Pack a chunk header into memory
 *
 * Takes a proto chunk and generates the header and length bytes.
 *
 * @param start The destination for the header
 * @param chunk The chunk being packed
 */
void chunk_make(uint8_t* start, chunk_t chunk);

/**
 * @brief Decode memory into a chunk
 *
 * Decodes a bit of memory known to contain chunked data and returns a struct
 * describing what was found. Note: if the chunk is nested (a chunk type) it
 * will not produce a
 * tree.
 *
 * @param start A pointer to uint8_t bytes making up an encoded chunk
 * @return A chunk structure describing the data
 */
chunk_t chunk_decode(uint8_t* start);

/**
 * @brief Get the N'th element from a set
 *
 * Gets the zero-indexed N'th item in a set type. If the item was found it
 * returns 1, else 0. If the chunk is not a set it returns 0.
 *
 * @param chunk A chunk
 * @param dest If found the chunk will be copied here
 * @return 1 or 0
 */
uint8_t chunk_set_get_nth(chunk_t chunk, chunk_t* dest, uint64_t nth);

/**
 * @brief Get number of elements in set
 *
 * @param chunk A chunk
 * @return Number of items in set
 */
uint64_t chunk_set_nr_items(chunk_t chunk);

/**
 * @brief Total byte length of a chunk
 *
 * Returns the total number of bytes in the chunk, including the header.
 *
 * @param chunk A chunk
 * @return Number of bytes for that chunk
 */
uint64_t chunk_total_length(chunk_t chunk);

#endif

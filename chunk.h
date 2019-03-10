#ifndef H_CHUNK
#define H_CHUNK

#include <stdint.h>

typedef enum chunk_type {
    CHUNK_TYPE_UNDEF = 0x00,
    CHUNK_TYPE_UINT8 = 0x01,
    CHUNK_TYPE_INT8 = 0x02,
    CHUNK_TYPE_UINT16 = 0x03,
    CHUNK_TYPE_INT16 = 0x04,
    CHUNK_TYPE_UINT32 = 0x05,
    CHUNK_TYPE_INT32 = 0x06,
    CHUNK_TYPE_UINT64 = 0x07,
    CHUNK_TYPE_INT64 = 0x08,
    CHUNK_TYPE_FLOAT32 = 0x09,
    CHUNK_TYPE_FLOAT64 = 0x0a,
    CHUNK_TYPE_UTF8 = 0x0b,
    CHUNK_TYPE_REF = 0x0c,
    CHUNK_TYPE_SET = 0x0d
} chunk_type_t;

typedef struct chunk {
    uint8_t* address;
    chunk_type_t type;
    uint8_t nr_length_bytes;
    uint64_t total_length;
    uint64_t data_length;
    uint8_t* data;
} chunk_t;

/**
 * @brief Number of bytes for a given type
 *
 * Returns the number of bytes for the type of the chunk.
 *
 * @param chunk A chunk type
 * @return Number of bytes for that chunk type
 */
uint8_t chunk_bytes_per_type(chunk_type_t type);

/**
 * @brief Number of bytes for a given type
 *
 * Returns the number of bytes for the type of the chunk.
 *
 * @param chunk A chunk type
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
 * @brief Write a chunk header into a data address
 *
 * @param The destination for the header
 * @param The chunk type
 * @param The length of the data
 * @return Start of the data contents
 */
uint8_t* chunk_write_header(uint8_t* data, chunk_type_t type, uint64_t length);

/**
 * @brief Pack a chunk header into memory
 *
 * Takes a proto chunk and generates the header and length bytes.
 *
 * @param start The destination for the header
 * @param chunk The chunk being packed
 * @return Start of the data contents
 */
uint8_t* chunk_make(uint8_t* data, chunk_t chunk);

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
 * @brief Get the byte offset of an indexed item
 *
 * @param Starting chunk set
 * @param index
 * @return Byte offset
 */
uint64_t chunk_set_item_byte_offset(chunk_t chunk, uint32_t idx);

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
uint8_t chunk_set_get_nth(uint8_t* data, chunk_t* dest, uint64_t nth);

/**
 * @brief Get the byte offset of an indexed item
 *
 * @param Starting chunk address
 * @param List of 32 bit indexes
 * @param Number of indexes in that list
 * @return Byte offset
 */
uint64_t chunk_byte_offset(uint8_t* data, uint32_t* idx, uint32_t nr_idx);

/**
 * @brief Get number of elements in set
 *
 * @param chunk A chunk
 * @return Number of items in set
 */
uint64_t chunk_set_nr_items(chunk_t chunk);

#endif

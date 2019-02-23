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
    CHUNK_TYPE_REF,
    CHUNK_TYPE_SET
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
 * @brief Memory move instruction
 *
 * A list of these is populated by chunk_insert(). Since the caller
 * knows the depth of the tree for the insert (chunk_insert_t: location
 * and depth) the caller is also responsible for creating a list of
 * empty chunk_move_t that chunk_insert() will fill with operations.
 * This list should always have at least 1 chunk_move_t populated (as
 * long as the location is valid) which is the move to make space for
 * the new inserted item. However additional moves may also be needed in
 * order to extend length bytes for parent set objects.
 *
 * The caller is responsible for iterating over this list and executing
 * the memory move operations according to the spec:
 *
 *   src:  The byte offset from the root where the data being moved
 *         starts.
 *   size: The size of the move, or how many bytes forward the block of
 *         memory should be shifted.
 *   data: A block of bytes of length move_size that will be written
 *         into the space between move_src and move_src + move_size.
 *
 * Note: the length of the block of memory being shifted is not provided
 * because it is always the total number of bytes from move_src to the
 * end of the root memory block, and should be known to the caller. An
 * example of using a list of moves in combination with memmove and
 * memcpy:
 *
 *   for (uint32_t i = 0; i < insert.depth; i++) {
 *       chunk_move_t move = insert.moves[i];
 *       uint64_t size = total_length - move.src;
 *       memmove(&dat[move.src + move.size], &dat[move.src], size);
 *       memcpy(&dat[move.src], move.data, move.size);
 *   }
 *
 */
typedef struct chunk_move {
    uint64_t start;
    uint64_t length;
} chunk_move_t;

typedef struct chunk_lb_shift {
    uint64_t chunk_start;
    uint64_t length;
    uint8_t nr_length_bytes;
    uint8_t length_bytes[16];
} chunk_lb_shift_t;

/**
 * @brief Insert instruction
 *
 * This struct is used to tell chunk about a new data insert. You have
 * to make one of these structs before passing it to chunk_insert(). You
 * then inspect it afterwards for instructions on how to modify the data
 * structure in order to insert - a list of #chunk_move_t objects.
 */
typedef struct chunk_insert {
    uint8_t* data;
    uint64_t size;
    uint32_t* location;
    uint32_t depth;
    uint32_t idx;
    chunk_move_t* moves;
} chunk_insert_t;

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

void foo(uint8_t* data, uint64_t length, uint32_t* idx, uint32_t nr_idx);

/**
 * @brief Build an insert instruction list
 *
 * @param Insert record to be populated
 */
void chunk_set_insert(chunk_insert_t* insert);

/**
 * @brief Get number of elements in set
 *
 * @param chunk A chunk
 * @return Number of items in set
 */
uint64_t chunk_set_nr_items(chunk_t chunk);

#endif

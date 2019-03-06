#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
#include "chunk.h"

#define CHUNK_COLOR_SET 1
#define CHUNK_COLOR_ITEM 2

//                   __ Item index 1 of the set, which is index 2 of the
//                  /   root set.
//                 /
// S[I..I...S[I...I....I]]
//
// Keypad left does:
//
//   Decrements the cursor_path_idx. Regenerates cursor_byte_offset from the
//   path.
//
// Keypad right does:
//
//   If the current object is a set:
//     Increments cursor_path_idx and sets cursor_path[cursor_path_idx] to 0 if
//     the current object. Regenerates cursor_byte_offset.
//   If the current object is an item:
//     Nothing yet, but could browse through data
//
// Keypad up:
//   If cursor_path[cursor_path_idx-1] is a set:
//     Decrement cursor_path[cursor_path_idx]. Do not decrement past zero.
//     Regenerate cursor_byte_offset.
//
// Keypad down:
//
//   If cursor_path[cursor_path_idx-1] is a set:
//     Increment cursor_path[cursor_path_idx]. Check parent set for end of set
//     index condition. Regenerate cursor_byte_offset.
//
// When rendering:
//   If cursor_byte_offset is equal to the start of the current chunk
//   highlight it.
//
typedef struct c_context {
    int fd;
    uint8_t* start;
    uint32_t cursor_path[256];
    uint8_t cursor_path_idx;
    uint64_t cursor_byte_offset;
} c_context_t;

// uint64_t chunk_byte_offset(uint8_t* data, uint32_t* idx, uint32_t nr_idx) {

static const char* name_per_type[] = {
    "X",
    "I",
    "i",
    "I",
    "i",
    "I",
    "i",
    "I",
    "i",
    "f",
    "f",
    "s",
    "R",
    "["
};

void draw_box(uint8_t xoff, uint8_t yoff, uint8_t w, uint8_t h) {
    uint8_t x, y;
    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) {
            mvaddch(y + yoff, x + xoff, ACS_CKBOARD);
        }
    }
}

void draw_set(c_context_t* context, chunk_t chunk, uint8_t xoff, uint8_t yoff) {
    attron(COLOR_PAIR(CHUNK_COLOR_SET));
    draw_box(xoff, yoff, 10, 1);
    mvprintw(yoff, xoff, "%s", name_per_type[chunk.type]);
    attroff(COLOR_PAIR(CHUNK_COLOR_SET));
}

void draw_item(c_context_t* context, chunk_t chunk, uint8_t xoff, uint8_t yoff) {
    uint8_t highlight = 0;
    //if (context->set_index == context->cursor_set_index && context->current_depth == context->cursor_depth) {
    //    highlight = 2;
    //}
    uint8_t bytes_per_type = chunk_bytes_per_type(chunk.type);
    uint64_t nr_items = chunk.data_length / bytes_per_type;
    attron(COLOR_PAIR(CHUNK_COLOR_ITEM + highlight));
    draw_box(xoff, yoff, 10, 1);
    mvprintw(yoff, xoff, "%s:%lu", name_per_type[chunk.type], nr_items);
    attroff(COLOR_PAIR(CHUNK_COLOR_ITEM + highlight));
}

uint8_t draw_chunk(c_context_t* context, chunk_t chunk, uint8_t xoff, uint8_t yoff) {
    if (chunk.type == CHUNK_TYPE_SET) {
        draw_set(context, chunk, xoff, yoff);
        uint8_t this_height = 1;
        uint64_t remaining = chunk.data_length;
        uint8_t* data = chunk.data;
        //context->set_index = 0;
        while (remaining) {
            chunk_t child = chunk_decode(data);
            //context->current_depth++;
            this_height += draw_chunk(context, child, xoff + 1, yoff + this_height);
            //context->current_depth--;
            data = data + child.total_length;
            remaining = remaining - child.total_length;
            //context->set_index++;
        }
        return this_height;
    }

    draw_item(context, chunk, xoff, yoff);
    return 1;
}

void load_file(c_context_t* context, const char* file) {
    uint8_t head[9];
    int fd = open(file, O_RDONLY);

    if (fd == -1) {
        printf("Error opening file\n");
        return;
    }

    size_t count = read(fd, head, 9);
    if (count < 2) {
        printf("Error reading from file\n");
        return;
    }

    chunk_t chunk = chunk_decode(head);

    if (count < 9) {
        close(fd);
        draw_chunk(context, chunk, 1, 0);
        return;
    }

    lseek(fd, 0, SEEK_SET);
    context->start = mmap(0, chunk.total_length, PROT_READ, MAP_SHARED, fd, 0);
    if (context->start == MAP_FAILED) {
        close(fd);
        mvprintw(0, 0, "mmap failed!");
        return;
    }

    context->fd = fd;
    return;
}

void loop(c_context_t* context) {
    uint8_t running = 1;
    uint8_t render = 1;
    while (running) {
        int c = getch();
        switch (c) {
            case KEY_UP:
                render = 1;
                break;
            case KEY_RIGHT:
                render = 1;
                break;
            default:
                break;
        }
        if (render) {
            chunk_t chunk = chunk_decode(context->start);
            draw_chunk(context, chunk, 1, 1);
            refresh();
            render = 0;
        }
    }
}

void init_context(c_context_t* context) {
    context->fd = -1;
    //context->cursor_depth = 1;
    //context->cursor_set_index = 0;
    //context->set_index = 0;
    //context->current_depth = 0;
}

void initcolors() {
    if (!has_colors()) {
        endwin();
        printf("Your terminal does not support color\n");
        exit(1);
    }
    start_color();
    init_pair(1, COLOR_BLACK, COLOR_GREEN);
    init_pair(2, COLOR_BLACK, COLOR_BLUE);
    init_pair(3, COLOR_GREEN, COLOR_BLACK);
    init_pair(4, COLOR_BLUE, COLOR_BLACK);
}

void init() {
    initscr();
    initcolors();
    noecho();
    keypad(stdscr, TRUE);
}

void deinit() {
    endwin();
}

int main(int argc, char* argv[]) {
    init();
    c_context_t context;
    init_context(&context);
    load_file(&context, "test.hpd");
    loop(&context);
    deinit();
    return 1;
}

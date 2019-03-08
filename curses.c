#include <ncurses.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
#include "chunk.h"
#include "chunk_node.h"

#define CHUNK_COLOR_SET 1
#define CHUNK_COLOR_ITEM 2

typedef struct c_context {
    int fd;
    chunk_node_t* root;
    uint32_t cursor_path[256];
    uint8_t cursor_path_idx;
    uint64_t cursor_byte_offset;
    uint64_t current_byte_offset;
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

void draw_set(c_context_t* context, chunk_node_t* node, uint8_t xoff, uint8_t yoff) {
    uint8_t highlight = 0;
    if (context->current_byte_offset == context->cursor_byte_offset) {
        highlight = 2;
    }
    attron(COLOR_PAIR(CHUNK_COLOR_SET + highlight));
    draw_box(xoff, yoff, 2, 1);
    mvprintw(yoff, xoff, "%s", name_per_type[node->type]);
    attroff(COLOR_PAIR(CHUNK_COLOR_SET + highlight));
}

void draw_item(c_context_t* context, chunk_node_t* node, uint8_t xoff, uint8_t yoff) {
    uint8_t highlight = 0;
    if (context->current_byte_offset == context->cursor_byte_offset) {
        highlight = 2;
    }
    uint8_t bytes_per_type = chunk_bytes_per_type(node->type);
    uint64_t nr_items = node->data_length / bytes_per_type;
    attron(COLOR_PAIR(CHUNK_COLOR_ITEM + highlight));
    draw_box(xoff, yoff, 3, 1);
    mvprintw(yoff, xoff, "%s:%lu", name_per_type[node->type], nr_items);
    attroff(COLOR_PAIR(CHUNK_COLOR_ITEM + highlight));
}

uint8_t draw_chunk_node(c_context_t* context, chunk_node_t* node, uint8_t xoff, uint8_t yoff) {
    if (node->type == CHUNK_TYPE_SET) {
        draw_set(context, node, xoff, yoff);
        uint8_t this_height = 1;
        for (uint64_t i = 0; i < node->nr_children; i++) {
            chunk_node_t* child = &node->children[i];
            this_height += draw_chunk_node(context, child, xoff + 1, yoff + this_height);
        }
        return this_height;
    }

    draw_item(context, node, xoff, yoff);
    return 1;
}

void draw(c_context_t* context, uint8_t xoff, uint8_t yoff) {
    draw_chunk_node(context, context->root, xoff, yoff);
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

    if (count < 9) {
        close(fd);
        context->root = chunk_node_build(head);
        //draw_chunk(context, root, 1, 0);
        return;
    }

    chunk_t chunk = chunk_decode(head);

    lseek(fd, 0, SEEK_SET);
    uint8_t* start = mmap(0, chunk.total_length, PROT_READ, MAP_SHARED, fd, 0);
    if (start == MAP_FAILED) {
        close(fd);
        mvprintw(0, 0, "mmap failed!");
        return;
    }

    context->root = chunk_node_build(start);
    context->fd = fd;
    return;
}

void regenerate_cursor_offset(c_context_t* context) {
    //context->cursor_byte_offset = chunk_byte_offset(context->start, context->cursor_path, (uint32_t)context->cursor_path_idx + 1);
    mvprintw(0, 0, "%lu   ", context->cursor_byte_offset);
}

uint8_t key_up(c_context_t* context) {
    if (context->cursor_path[context->cursor_path_idx] == 0) {
        return 0;
    }
    context->cursor_path[context->cursor_path_idx]--;
    regenerate_cursor_offset(context);
    return 1;
}

uint8_t key_down(c_context_t* context) {
    context->cursor_path[context->cursor_path_idx]++;
    regenerate_cursor_offset(context);
    // As I don't have an indication that the index exceeds the length of the
    // set, looking for a zero here is an expensive way of backing off the end
    // of the set.
    if (context->cursor_byte_offset == 0) {
        context->cursor_path[context->cursor_path_idx]--;
        regenerate_cursor_offset(context);
    }
    return 1;
}

uint8_t key_left(c_context_t* context) {
    if (context->cursor_path_idx == 0) {
        return 0;
    }
    context->cursor_path_idx--;
    regenerate_cursor_offset(context);
    return 1;
}

uint8_t key_right(c_context_t* context) {
/*
    chunk_t chunk = chunk_decode(context->start + context->cursor_byte_offset);
    if (chunk.type == CHUNK_TYPE_SET) {
        context->cursor_path_idx++;
        context->cursor_path[context->cursor_path_idx] = 0;
        regenerate_cursor_offset(context);
        return 1;
    }
*/
    return 0;
}

void loop(c_context_t* context) {
    uint8_t running = 1;
    uint8_t render = 1;
    while (running) {
        int c = getch();
        switch (c) {
            case KEY_UP:
                render = key_up(context);
                break;
            case KEY_DOWN:
                render = key_down(context);
                break;
            case KEY_LEFT:
                render = key_left(context);
                break;
            case KEY_RIGHT:
                render = key_right(context);
                break;
            default:
                break;
        }
        if (render) {
            draw(context, 1, 1);
            refresh();
            render = 0;
        }
    }
}

void init_context(c_context_t* context) {
    context->fd = -1;
    memset(context->cursor_path, 0, 256);
    context->cursor_path_idx = 0;
    context->cursor_byte_offset = 0;
    context->current_byte_offset = 0;
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

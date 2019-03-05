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

typedef struct ccontext {
    uint32_t cursor_depth;
    uint32_t cursor_set_index;
    uint32_t set_index;
    uint32_t current_depth;
} ccontext_t;

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

void draw_set(ccontext_t* context, chunk_t chunk, uint8_t xoff, uint8_t yoff) {
    attron(COLOR_PAIR(CHUNK_COLOR_SET));
    draw_box(xoff, yoff, 10, 1);
    mvprintw(yoff, xoff, "%s", name_per_type[chunk.type]);
    attroff(COLOR_PAIR(CHUNK_COLOR_SET));
}

void draw_item(ccontext_t* context, chunk_t chunk, uint8_t xoff, uint8_t yoff) {
    uint8_t highlight = 0;
    if (context->set_index == context->cursor_set_index) {
        highlight = 2;
    }
    attron(COLOR_PAIR(CHUNK_COLOR_ITEM + highlight));
    draw_box(xoff, yoff, 10, 1);
    mvprintw(yoff, xoff, "%s", name_per_type[chunk.type]);
    attroff(COLOR_PAIR(CHUNK_COLOR_ITEM + highlight));
}

uint8_t draw_chunk(ccontext_t* context, chunk_t chunk, uint8_t xoff, uint8_t yoff) {
    if (chunk.type == CHUNK_TYPE_SET) {
        draw_set(context, chunk, xoff, yoff);
        uint8_t this_height = 1;
        uint64_t remaining = chunk.data_length;
        uint8_t* data = chunk.data;
        uint32_t count = 0;
        while (remaining) {
            chunk_t child = chunk_decode(data);
            context->current_depth++;
            context->set_index = count;
            this_height += draw_chunk(context, child, xoff + 1, yoff + this_height);
            context->current_depth--;
            data = data + child.total_length;
            remaining = remaining - child.total_length;
            count++;
        }
        return this_height;
    }

    draw_item(context, chunk, xoff, yoff);
    return 1;
}

void draw_file(ccontext_t* context, const char* file) {
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
        draw_chunk(context, chunk, 1, 0);
        return;
    }

    lseek(fd, 0, SEEK_SET);
    uint8_t* start = mmap(0, chunk.total_length, PROT_READ, MAP_SHARED, fd, 0);
    if (start == MAP_FAILED) {
        close(fd);
        mvprintw(0, 0, "mmap failed!");
        return;
    }

    chunk = chunk_decode(start);
    draw_chunk(context, chunk, 1, 1);
    mvprintw(0, 0, "data_length: %lu", chunk.total_length);
    return;
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
}

void deinit() {
    endwin();
}

void init_context(ccontext_t* context) {
    context->cursor_depth = 0;
    context->cursor_set_index = 0;
    context->set_index = 0;
    context->current_depth = 0;
}

int main(int argc, char* argv[]) {
    init();
    ccontext_t context;
    init_context(&context);
    context.cursor_depth = 0;
    draw_file(&context, "test.hpd");
    refresh();
    getch();
    deinit();
    return 1;
}

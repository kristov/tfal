#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
#include "chunk.h"

void draw_box(uint8_t xoff, uint8_t yoff, uint8_t w, uint8_t h, uint8_t c) {
    attron(COLOR_PAIR(c));
    uint8_t x, y;
    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) {
            mvaddch(y + yoff, x + xoff, ' ');
        }
    }
    mvaddch(yoff, xoff, '[');
    attroff(COLOR_PAIR(c));
}

uint8_t draw_chunk(chunk_t chunk, uint8_t xoff, uint8_t yoff) {
    if (chunk.type == CHUNK_TYPE_SET) {
        draw_box(xoff, yoff, 10, 2, 1);
        yoff++;
        uint64_t remaining = chunk.data_length;
        uint8_t* data = chunk.data;
        while (remaining) {
            chunk_t child = chunk_decode(data);
            yoff += draw_chunk(child, xoff + 1, yoff);
            data = data + child.total_length;
            remaining = remaining - child.total_length;
        }
        return yoff;
    }
    else {
        draw_box(xoff, yoff, 10, 1, 2);
        mvprintw(yoff, xoff, "[%d]", chunk.type);
        return 1;
    }
    return 0;
}

void draw_file() {
    uint8_t head[9];
    int fd = open("test.hpd", O_RDONLY);

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
        draw_chunk(chunk, 1, 0);
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
    draw_chunk(chunk, 1, 1);
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
}

void init() {
    initscr();
    initcolors();
}

void deinit() {
    endwin();
}

int main(int argc, char* argv[]) {
    init();
    draw_file();
    refresh();
    getch();
    deinit();
    return 1;
}

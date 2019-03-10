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

#define CHUNK_COLOR_DATA 1
#define CHUNK_COLOR_SET 2
#define CHUNK_COLOR_ITEM 3

typedef struct c_context {
    int fd;
    chunk_node_t* root;
    uint64_t cursor_path[256];
    uint8_t cursor_path_idx;
    uint8_t tabstop;
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

void draw_item_uint8(c_context_t* context, chunk_node_t* node, uint8_t xoff, uint8_t yoff) {
    attron(COLOR_PAIR(CHUNK_COLOR_DATA));
    char num[4];
    uint8_t* data = (uint8_t*)node->data;
    for (uint8_t i = 0; i < node->nr_children; i++) {
        uint8_t v = data[i];
        sprintf(num, "%u", v);
        mvprintw(yoff, xoff, "%s", num);
        xoff += strlen(num) + 1;
    }
    attroff(COLOR_PAIR(CHUNK_COLOR_DATA));
}

void draw_item_float64(c_context_t* context, chunk_node_t* node, uint8_t xoff, uint8_t yoff) {
    attron(COLOR_PAIR(CHUNK_COLOR_DATA));
    char num[2048];
    double* data = (double*)node->data;
    for (uint8_t i = 0; i < node->nr_children; i++) {
        double v = data[i];
        sprintf(num, "%lf", v);
        mvprintw(yoff, xoff, "%s", num);
        xoff += strlen(num) + 1;
    }
    attroff(COLOR_PAIR(CHUNK_COLOR_DATA));
}

void draw_item_data(c_context_t* context, chunk_node_t* node, uint8_t xoff, uint8_t yoff) {
    switch (node->type) {
        case CHUNK_TYPE_UINT8:
            draw_item_uint8(context, node, xoff, yoff);
            break;
        case CHUNK_TYPE_INT8:
            break;
        case CHUNK_TYPE_UINT16:
            break;
        case CHUNK_TYPE_INT16:
            break;
        case CHUNK_TYPE_UINT32:
            break;
        case CHUNK_TYPE_INT32:
            break;
        case CHUNK_TYPE_UINT64:
            break;
        case CHUNK_TYPE_INT64:
            break;
        case CHUNK_TYPE_FLOAT32:
            break;
        case CHUNK_TYPE_FLOAT64:
            draw_item_float64(context, node, xoff, yoff);
            break;
        case CHUNK_TYPE_UTF8:
            break;
        case CHUNK_TYPE_REF:
            break;
        default:
            break;
    }
}

void draw_item(c_context_t* context, chunk_node_t* node, uint8_t xoff, uint8_t yoff) {
    char nr_children[21];
    char bytes_per_type[4];
    sprintf(nr_children, "%lu", node->nr_children);
    sprintf(bytes_per_type, "%u", node->bytes_per_type);
    uint8_t highlight = 0;
    if (node->selected) {
        highlight = 3;
    }
    attron(COLOR_PAIR(CHUNK_COLOR_ITEM + highlight));
    draw_box(xoff, yoff, 3, 1);
    mvprintw(yoff, xoff, "%s:%s:%s", name_per_type[node->type], nr_children, bytes_per_type);
    attroff(COLOR_PAIR(CHUNK_COLOR_ITEM + highlight));
    uint8_t head_len = strlen(nr_children) + strlen(bytes_per_type) + 4;
    draw_item_data(context, node, xoff + head_len, yoff);
}

void draw_set(c_context_t* context, chunk_node_t* node, uint8_t xoff, uint8_t yoff) {
    uint8_t highlight = 0;
    if (node->selected) {
        highlight = 3;
    }
    attron(COLOR_PAIR(CHUNK_COLOR_SET + highlight));
    draw_box(xoff, yoff, 2, 1);
    mvprintw(yoff, xoff, "%s:%lu", name_per_type[13], node->nr_children);
    attroff(COLOR_PAIR(CHUNK_COLOR_SET + highlight));
}

uint8_t draw_chunk_node(c_context_t* context, chunk_node_t* node, uint8_t xoff, uint8_t yoff) {
    if (node->type == CHUNK_TYPE_SET) {
        draw_set(context, node, xoff, yoff);
        uint8_t this_height = 1;
        for (uint64_t i = 0; i < node->nr_children; i++) {
            chunk_node_t* child = &node->children[i];
            this_height += draw_chunk_node(context, child, xoff + context->tabstop, yoff + this_height);
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
        draw(context, 1, 0);
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
    draw(context, 1, 1);
    return;
}

uint8_t key_up(c_context_t* context) {
    if (context->cursor_path[context->cursor_path_idx] == 0) {
        return 0;
    }
    chunk_node_t* prev = chunk_node_select(context->root, context->cursor_path, context->cursor_path_idx + 1);
    if (prev == NULL) {
        return 0;
    }
    prev->selected = 0;
    context->cursor_path[context->cursor_path_idx]--;
    chunk_node_t* next = chunk_node_select(context->root, context->cursor_path, context->cursor_path_idx + 1);
    if (next == NULL) {
        context->cursor_path[context->cursor_path_idx]++;
        prev->selected = 1;
        return 0;
    }
    next->selected = 1;
    return 1;
}

uint8_t key_down(c_context_t* context) {
    chunk_node_t* prev = chunk_node_select(context->root, context->cursor_path, context->cursor_path_idx + 1);
    if (prev == NULL) {
        return 0;
    }
    prev->selected = 0;
    context->cursor_path[context->cursor_path_idx]++;
    chunk_node_t* next = chunk_node_select(context->root, context->cursor_path, context->cursor_path_idx + 1);
    if (next == NULL) {
        context->cursor_path[context->cursor_path_idx]--;
        prev->selected = 1;
        return 0;
    }
    next->selected = 1;
    return 1;
}

uint8_t key_left(c_context_t* context) {
    if (context->cursor_path_idx == 0) {
        return 0;
    }
    chunk_node_t* prev = chunk_node_select(context->root, context->cursor_path, context->cursor_path_idx + 1);
    if (prev == NULL) {
        return 0;
    }
    prev->selected = 0;
    context->cursor_path_idx--;
    chunk_node_t* next = chunk_node_select(context->root, context->cursor_path, context->cursor_path_idx + 1);
    if (next == NULL) {
        context->cursor_path_idx++;
        prev->selected = 1;
        return 0;
    }
    next->selected = 1;
    return 1;
}

uint8_t key_right(c_context_t* context) {
    chunk_node_t* prev = chunk_node_select(context->root, context->cursor_path, context->cursor_path_idx + 1);
    if (prev == NULL) {
        return 0;
    }
    if (prev->type != CHUNK_TYPE_SET) {
        return 0;
    }
    prev->selected = 0;
    context->cursor_path_idx++;
    context->cursor_path[context->cursor_path_idx] = 0;
    chunk_node_t* next = chunk_node_select(context->root, context->cursor_path, context->cursor_path_idx + 1);
    if (next == NULL) {
        context->cursor_path_idx--;
        prev->selected = 1;
        return 0;
    }
    next->selected = 1;
    return 1;
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
    context->tabstop = 2;
    memset(context->cursor_path, 0, 256);
    context->cursor_path_idx = 0;
}

void initcolors() {
    if (!has_colors()) {
        endwin();
        printf("Your terminal does not support color\n");
        exit(1);
    }
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_BLUE, COLOR_BLACK);

    init_pair(4, COLOR_BLACK, COLOR_WHITE);
    init_pair(5, COLOR_BLACK, COLOR_GREEN);
    init_pair(6, COLOR_BLACK, COLOR_BLUE);
}

void init() {
    initscr();
    initcolors();
    noecho();
    curs_set(0);
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

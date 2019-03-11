#include <ncursesw/ncurses.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
#include <locale.h>

#include "chunk.h"
#include "chunk_node.h"

#define CHUNK_COLOR_DATA 0x0e
#define CHUNK_COLOR_HIGHLIGHT 0x10

typedef enum curses_mode {
    CURSES_MODE_MOVE = 0x01,
    CURSES_MODE_INSERT = 0x02
} curses_mode_t;

typedef struct c_context {
    int fd;
    chunk_node_t* root;
    curses_mode_t mode;
    uint64_t cursor_path[256];
    uint8_t cursor_path_idx;
    uint64_t item_idx;
    uint8_t tabstop;
} c_context_t;

static const char name_per_type[] = {
    'X',
    'I',
    'i',
    'I',
    'i',
    'I',
    'i',
    'I',
    'i',
    'f',
    'f',
    's',
    'R',
    '['
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
    if (node->selected) {
        for (uint8_t i = 0; i < node->nr_children; i++) {
            uint8_t v = data[i];
            sprintf(num, "%u", v);
            if (i == context->item_idx) {
                attroff(COLOR_PAIR(CHUNK_COLOR_DATA));
                attron(COLOR_PAIR(CHUNK_COLOR_DATA + CHUNK_COLOR_HIGHLIGHT));
                mvprintw(yoff, xoff, "%s", num);
                attroff(COLOR_PAIR(CHUNK_COLOR_DATA + CHUNK_COLOR_HIGHLIGHT));
                attron(COLOR_PAIR(CHUNK_COLOR_DATA));
            }
            else {
                mvprintw(yoff, xoff, "%s", num);
            }
            xoff += strlen(num) + 1;
        }
    }
    else {
        for (uint8_t i = 0; i < node->nr_children; i++) {
            uint8_t v = data[i];
            sprintf(num, "%u", v);
            mvprintw(yoff, xoff, "%s", num);
            xoff += strlen(num) + 1;
        }
    }
    attroff(COLOR_PAIR(CHUNK_COLOR_DATA));
}

void draw_item_float64(c_context_t* context, chunk_node_t* node, uint8_t xoff, uint8_t yoff) {
    attron(COLOR_PAIR(CHUNK_COLOR_DATA));
    char num[2048];
    double* data = (double*)node->data;
    if (node->selected) {
        for (uint8_t i = 0; i < node->nr_children; i++) {
            double v = data[i];
            sprintf(num, "%lf", v);
            if (i == context->item_idx) {
                attroff(COLOR_PAIR(CHUNK_COLOR_DATA));
                attron(COLOR_PAIR(CHUNK_COLOR_DATA + CHUNK_COLOR_HIGHLIGHT));
                mvprintw(yoff, xoff, "%s", num);
                attroff(COLOR_PAIR(CHUNK_COLOR_DATA + CHUNK_COLOR_HIGHLIGHT));
                attron(COLOR_PAIR(CHUNK_COLOR_DATA));
            }
            else {
                mvprintw(yoff, xoff, "%s", num);
            }
            xoff += strlen(num) + 1;
        }
    }
    else {
        for (uint8_t i = 0; i < node->nr_children; i++) {
            double v = data[i];
            sprintf(num, "%lf", v);
            mvprintw(yoff, xoff, "%s", num);
            xoff += strlen(num) + 1;
        }
    }
    attroff(COLOR_PAIR(CHUNK_COLOR_DATA));
}

void draw_item_utf8(c_context_t* context, chunk_node_t* node, uint8_t xoff, uint8_t yoff) {
    attron(COLOR_PAIR(CHUNK_COLOR_DATA));
    uint8_t str[256];
    memcpy(str, node->data, 255);
    str[255] = '\0';
    if (node->nr_children < 255) {
        str[node->nr_children] = '\0';
    }
    mvprintw(yoff, xoff, "%s", str);
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
            draw_item_utf8(context, node, xoff, yoff);
            break;
        case CHUNK_TYPE_REF:
            break;
        default:
            break;
    }
}

void draw_item(c_context_t* context, chunk_node_t* node, uint8_t xoff, uint8_t yoff) {
    char head[24]; // I1:45265
    memset(head, 0, 24);
    sprintf(head, "%c%u:%lu", name_per_type[node->type], node->bytes_per_type, node->nr_children);
    uint8_t highlight = 0;
    if (node->selected) {
        highlight = CHUNK_COLOR_HIGHLIGHT;
    }
    attron(COLOR_PAIR(node->type + highlight));
    draw_box(xoff, yoff, 3, 1);
    mvprintw(yoff, xoff, "%s", head);
    attroff(COLOR_PAIR(node->type + highlight));
    uint8_t head_len = strlen(head) + 1;
    draw_item_data(context, node, xoff + head_len, yoff);
}

void draw_set(c_context_t* context, chunk_node_t* node, uint8_t xoff, uint8_t yoff) {
    uint8_t highlight = 0;
    if (node->selected) {
        highlight = CHUNK_COLOR_HIGHLIGHT;
    }
    attron(COLOR_PAIR(node->type + highlight));
    draw_box(xoff, yoff, 2, 1);
    mvprintw(yoff, xoff, "%c:%lu", name_per_type[13], node->nr_children);
    attroff(COLOR_PAIR(node->type + highlight));
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

/*
uint8_t key_left_item(c_context_t* context, chunk_node_t* curr) {
}

uint8_t key_left_set(c_context_t* context, chunk_node_t* curr) {
}
*/

uint8_t key_left(c_context_t* context) {
    if (context->cursor_path_idx == 0) {
        return 0;
    }
    chunk_node_t* curr = chunk_node_select(context->root, context->cursor_path, context->cursor_path_idx + 1);
    if (curr == NULL) {
        return 0;
    }
    curr->selected = 0;
    context->cursor_path_idx--;
    chunk_node_t* next = chunk_node_select(context->root, context->cursor_path, context->cursor_path_idx + 1);
    if (next == NULL) {
        context->cursor_path_idx++;
        curr->selected = 1;
        return 0;
    }
    next->selected = 1;
    return 1;
}

uint8_t key_right_item(c_context_t* context, chunk_node_t* curr) {
    if ((context->item_idx + 1) >= curr->nr_children) {
        context->item_idx = 0;
        return 1;
    }
    context->item_idx++;
    return 1;
}

uint8_t key_right_set(c_context_t* context, chunk_node_t* curr) {
    curr->selected = 0;
    context->cursor_path_idx++;
    context->cursor_path[context->cursor_path_idx] = 0;
    chunk_node_t* next = chunk_node_select(context->root, context->cursor_path, context->cursor_path_idx + 1);
    if (next == NULL) {
        context->cursor_path_idx--;
        curr->selected = 1;
        return 0;
    }
    next->selected = 1;
    return 1;
}

uint8_t key_right(c_context_t* context) {
    chunk_node_t* curr = chunk_node_select(context->root, context->cursor_path, context->cursor_path_idx + 1);
    if (curr == NULL) {
        return 0;
    }
    if (curr->type == CHUNK_TYPE_SET) {
        return key_right_set(context, curr);
    }
    return key_right_item(context, curr);
}

uint8_t handle_mode_move(c_context_t* context, int c) {
    uint8_t render = 1;
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
        case 'i':
            context->mode = CURSES_MODE_INSERT;
            break;
        default:
            break;
    }
    return render;
}

void loop(c_context_t* context) {
    uint8_t running = 1;
    uint8_t render = 1;
    while (running) {
        int c = getch();
        switch (context->mode) {
            case CURSES_MODE_MOVE:
                render = handle_mode_move(context, c);
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
    context->mode = CURSES_MODE_MOVE;
    memset(context->cursor_path, 0, 256);
    context->cursor_path_idx = 0;
    context->item_idx = 0;
}

void initcolors() {
    if (!has_colors()) {
        endwin();
        printf("Your terminal does not support color\n");
        exit(1);
    }

    start_color();
    init_pair(0x01, COLOR_BLUE, COLOR_BLACK);
    init_pair(0x02, COLOR_BLUE, COLOR_BLACK);
    init_pair(0x03, COLOR_BLUE, COLOR_BLACK);
    init_pair(0x04, COLOR_BLUE, COLOR_BLACK);
    init_pair(0x05, COLOR_BLUE, COLOR_BLACK);
    init_pair(0x06, COLOR_BLUE, COLOR_BLACK);
    init_pair(0x07, COLOR_BLUE, COLOR_BLACK);
    init_pair(0x08, COLOR_BLUE, COLOR_BLACK);
    init_pair(0x09, COLOR_CYAN, COLOR_BLACK);
    init_pair(0x0a, COLOR_CYAN, COLOR_BLACK);
    init_pair(0x0b, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(0x0c, COLOR_YELLOW, COLOR_BLACK);
    init_pair(0x0d, COLOR_GREEN, COLOR_BLACK);
    init_pair(0x0e, COLOR_WHITE, COLOR_BLACK);

    init_pair(0x11, COLOR_BLACK, COLOR_BLUE);
    init_pair(0x12, COLOR_BLACK, COLOR_BLUE);
    init_pair(0x13, COLOR_BLACK, COLOR_BLUE);
    init_pair(0x14, COLOR_BLACK, COLOR_BLUE);
    init_pair(0x15, COLOR_BLACK, COLOR_BLUE);
    init_pair(0x16, COLOR_BLACK, COLOR_BLUE);
    init_pair(0x17, COLOR_BLACK, COLOR_BLUE);
    init_pair(0x18, COLOR_BLACK, COLOR_BLUE);
    init_pair(0x19, COLOR_BLACK, COLOR_CYAN);
    init_pair(0x1a, COLOR_BLACK, COLOR_CYAN);
    init_pair(0x1b, COLOR_BLACK, COLOR_MAGENTA);
    init_pair(0x1c, COLOR_BLACK, COLOR_YELLOW);
    init_pair(0x1d, COLOR_BLACK, COLOR_GREEN);
    init_pair(0x1e, COLOR_BLACK, COLOR_WHITE);
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
    setlocale(LC_ALL, "");
    init();
    c_context_t context;
    init_context(&context);
    load_file(&context, "test.hpd");
    loop(&context);
    deinit();
    return 1;
}

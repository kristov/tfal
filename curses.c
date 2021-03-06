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
#include "utf8.h"
#include "bitwise.h"

#define CHUNK_COLOR_DATA 0x0e
#define CHUNK_COLOR_ERROR 0x0f
#define CHUNK_COLOR_WARN 0x10
#define CHUNK_COLOR_HIGHLIGHT 0x10

#define CURSOR_FLAG_IN_DATA 1

typedef enum curses_mode {
    CURSES_MODE_MOVE = 0x01,
    CURSES_MODE_TYPE = 0x02,
    CURSES_MODE_CMDINPUT = 0x03,
    CURSES_MODE_DATINPUT = 0x04
} curses_mode_t;

typedef struct c_context {
    int fd;
    chunk_node_t* root;
    curses_mode_t mode;
    uint64_t cursor_path[256];
    uint8_t cursor_path_idx;
    uint8_t flags;
    uint64_t item_idx;
    uint8_t tabstop;
    uint8_t cmd_buf[257];
    uint8_t cmd_buf_idx;
    uint8_t cmd_ctx;
} c_context_t;

static const char* name_per_type[] = {
    "?",
    "i8",
    "I8",
    "i16",
    "I16",
    "i32",
    "I32",
    "i64",
    "I64",
    "f32",
    "f64",
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
    if (BIT_TEST(context->flags, CURSOR_FLAG_IN_DATA) && BIT_TEST(node->flags, NODE_FLAG_FOCUS)) {
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
    double v = 0;
    attron(COLOR_PAIR(CHUNK_COLOR_DATA));
    char num[2048];
    double* data = (double*)node->data;
    if (BIT_TEST(context->flags, CURSOR_FLAG_IN_DATA) && BIT_TEST(node->flags, NODE_FLAG_FOCUS)) {
        for (uint8_t i = 0; i < node->nr_children; i++) {
            v = data[i];
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
            v = data[i];
            sprintf(num, "%lf", v);
            mvprintw(yoff, xoff, "%s", num);
            xoff += strlen(num) + 1;
        }
    }
    attroff(COLOR_PAIR(CHUNK_COLOR_DATA));
}

void draw_item_utf8(c_context_t* context, chunk_node_t* node, uint8_t xoff, uint8_t yoff) {
    int cn = 0;
    uint32_t next[2];
    char* s = (char*)node->data;
    attron(COLOR_PAIR(CHUNK_COLOR_DATA));
    if (BIT_TEST(context->flags, CURSOR_FLAG_IN_DATA) && BIT_TEST(node->flags, NODE_FLAG_FOCUS)) {
        for (uint8_t i = 0; i < node->nr_children; i++) {
            next[0] = u8_nextchar(s, &cn);
            next[1] = 0;
            if (i == context->item_idx) {
                attroff(COLOR_PAIR(CHUNK_COLOR_DATA));
                attron(COLOR_PAIR(CHUNK_COLOR_DATA + CHUNK_COLOR_HIGHLIGHT));
                mvprintw(yoff, xoff, "%S", next);
                attroff(COLOR_PAIR(CHUNK_COLOR_DATA + CHUNK_COLOR_HIGHLIGHT));
                attron(COLOR_PAIR(CHUNK_COLOR_DATA));
            }
            else {
                mvprintw(yoff, xoff, "%S", next);
            }
            xoff += 1;
        }
    }
    else {
        for (uint8_t i = 0; i < node->nr_children; i++) {
            next[0] = u8_nextchar(s, &cn);
            next[1] = 0;
            mvprintw(yoff, xoff, "%S", next);
            xoff += 1;
        }
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
            draw_item_utf8(context, node, xoff, yoff);
            break;
        case CHUNK_TYPE_REF:
            break;
        default:
            break;
    }
}

void draw_item(c_context_t* context, chunk_node_t* node, uint8_t xoff, uint8_t yoff) {
    char head[24];
    memset(head, 0, 24);
    sprintf(head, "%s:%lu", name_per_type[node->type], node->nr_children);
    uint8_t color = node->type;
    if (node->type == 0) {
        color = CHUNK_COLOR_ERROR;
    }
    if (BIT_TEST(node->flags, NODE_FLAG_FOCUS)) {
        color += CHUNK_COLOR_HIGHLIGHT;
    }
    attron(COLOR_PAIR(color));
    draw_box(xoff, yoff, 3, 1);
    mvprintw(yoff, xoff, "%s", head);
    attroff(COLOR_PAIR(color));
    uint8_t head_len = strlen(head);
    draw_item_data(context, node, xoff + head_len + 1, yoff);
}

void draw_set(c_context_t* context, chunk_node_t* node, uint8_t xoff, uint8_t yoff) {
    uint8_t highlight = 0;
    if (BIT_TEST(node->flags, NODE_FLAG_FOCUS)) {
        highlight = CHUNK_COLOR_HIGHLIGHT;
    }
    attron(COLOR_PAIR(node->type + highlight));
    draw_box(xoff, yoff, 2, 1);
    mvprintw(yoff, xoff, "[:%lu", node->nr_children);
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
    if (context->mode == CURSES_MODE_CMDINPUT) {
        attron(COLOR_PAIR(CHUNK_COLOR_WARN));
        mvprintw(0, 0, ":%s", context->cmd_buf);
        attroff(COLOR_PAIR(CHUNK_COLOR_WARN));
    }
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
    chunk_node_t* curr = chunk_node_select(context->root, context->cursor_path, context->cursor_path_idx + 1);
    if (curr == NULL) {
        return 0;
    }
    context->cursor_path[context->cursor_path_idx]--;
    chunk_node_t* next = chunk_node_select(context->root, context->cursor_path, context->cursor_path_idx + 1);
    if (next == NULL) {
        context->cursor_path[context->cursor_path_idx]++;
        return 0;
    }
    context->item_idx = 0;
    BIT_UNSET(context->flags, CURSOR_FLAG_IN_DATA);
    BIT_UNSET(curr->flags, NODE_FLAG_FOCUS);
    BIT_SET(next->flags, NODE_FLAG_FOCUS);
    next->flags |= (1 << 0);
    return 1;
}

uint8_t key_down(c_context_t* context) {
    chunk_node_t* curr = chunk_node_select(context->root, context->cursor_path, context->cursor_path_idx + 1);
    if (curr == NULL) {
        return 0;
    }
    context->cursor_path[context->cursor_path_idx]++;
    chunk_node_t* next = chunk_node_select(context->root, context->cursor_path, context->cursor_path_idx + 1);
    if (next == NULL) {
        context->cursor_path[context->cursor_path_idx]--;
        return 0;
    }
    context->item_idx = 0;
    BIT_UNSET(context->flags, CURSOR_FLAG_IN_DATA);
    BIT_UNSET(curr->flags, NODE_FLAG_FOCUS);
    BIT_SET(next->flags, NODE_FLAG_FOCUS);
    return 1;
}

uint8_t key_left_set(c_context_t* context, chunk_node_t* curr) {
    if (context->cursor_path_idx == 0) {
        return 0;
    }
    context->cursor_path_idx--;
    chunk_node_t* next = chunk_node_select(context->root, context->cursor_path, context->cursor_path_idx + 1);
    if (next == NULL) {
        context->cursor_path_idx++;
        return 0;
    }
    BIT_UNSET(curr->flags, NODE_FLAG_FOCUS);
    BIT_SET(next->flags, NODE_FLAG_FOCUS);
    return 1;
}

uint8_t key_left_item(c_context_t* context, chunk_node_t* curr) {
    if (BIT_TEST(context->flags, CURSOR_FLAG_IN_DATA)) {
        if (context->item_idx == 0) {
            BIT_UNSET(context->flags, CURSOR_FLAG_IN_DATA);
            return 1;
        }
        context->item_idx--;
        return 1;
    }
    return key_left_set(context, curr);
}

uint8_t key_left(c_context_t* context) {
    chunk_node_t* curr = chunk_node_select(context->root, context->cursor_path, context->cursor_path_idx + 1);
    if (curr == NULL) {
        return 0;
    }
    if (curr->type == CHUNK_TYPE_SET) {
        return key_left_set(context, curr);
    }
    return key_left_item(context, curr);
}

uint8_t key_right_set(c_context_t* context, chunk_node_t* curr) {
    context->cursor_path_idx++;
    context->cursor_path[context->cursor_path_idx] = 0;
    chunk_node_t* next = chunk_node_select(context->root, context->cursor_path, context->cursor_path_idx + 1);
    if (next == NULL) {
        context->cursor_path_idx--;
        return 0;
    }
    BIT_UNSET(curr->flags, NODE_FLAG_FOCUS);
    BIT_SET(next->flags, NODE_FLAG_FOCUS);
    return 1;
}

uint8_t key_right_item(c_context_t* context, chunk_node_t* curr) {
    if (!BIT_TEST(context->flags, CURSOR_FLAG_IN_DATA)) {
        BIT_SET(context->flags, CURSOR_FLAG_IN_DATA);
        context->item_idx = 0;
        return 1;
    }
    if ((context->item_idx + 1) >= curr->nr_children) {
        context->item_idx = 0;
        return 1;
    }
    context->item_idx++;
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

uint8_t item_insert_append(c_context_t* context, uint64_t at, chunk_type_t type) {
    chunk_node_t* curr = chunk_node_select(context->root, context->cursor_path, context->cursor_path_idx + 1);
    if (curr == NULL) {
        return 0;
    }

    chunk_node_t* parent = NULL;
    if (context->cursor_path_idx == 0) {
        parent = context->root;
    }
    else {
        parent = chunk_node_select(context->root, &context->cursor_path[context->cursor_path_idx - 1], context->cursor_path_idx);
    }
    if (parent == NULL) {
        mvprintw(0, 0, "parent element NULL, curr->type: %d", curr->type);
        return 0;
    }
    if (parent->type != CHUNK_TYPE_SET) {
        return 0;
    }

    BIT_UNSET(curr->flags, NODE_FLAG_FOCUS);
    chunk_node_t* new = chunk_node_set_insert(parent, at);
    if (new == NULL) {
        BIT_SET(curr->flags, NODE_FLAG_FOCUS);
        return 0;
    }
    new->type = type;
    context->cursor_path[context->cursor_path_idx] = at;
    BIT_SET(new->flags, NODE_FLAG_FOCUS);
    return 1;
}

uint8_t key_report_length(c_context_t* context) {
    uint64_t length = chunk_node_size(context->root);
    mvprintw(0, 0, "total length: %lu     ", length);
    return 0;
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
        case ':':
            context->mode = CURSES_MODE_CMDINPUT;
            break;
        default:
            break;
    }
    return render;
}

uint8_t interpret_insert_append(c_context_t* context, uint8_t append) {
/*
    uint8_t* str_starts[256];
    uint8_t idx = 0;
    for (uint8_t i = 0; i < context->cmd_buf_idx; i++) {
        if (context->cmd_buf[i] == 0x20) {
            context->cmd_buf[i] = '\0';
            str_starts[idx] = &context->cmd_buf[i + 1];
            idx++;
        }
    }
    //render = key_insert(context);
*/
    return 1;
}

uint8_t reset_buffer(c_context_t* context, uint8_t render, curses_mode_t mode) {
    context->mode = mode;
    memset(context->cmd_buf, 0, 257);
    context->cmd_buf_idx = 0;
    return render;
}

uint8_t interpret_command(c_context_t* context) {
    uint8_t append = 0;
    uint64_t at = context->cursor_path[context->cursor_path_idx];
    char *token = strtok((char*)context->cmd_buf, " ");
    switch (token[0]) {
        case 'i':
            append = 0;
            break;
        case 'a':
            append = 1;
            break;
        default:
            return reset_buffer(context, 0, CURSES_MODE_MOVE);
            break;
    }
    token[0] = 'X';
    if (strcmp(token, "Xi8") == 0) {
        item_insert_append(context, at + append, CHUNK_TYPE_UINT8);
    }
    if (strcmp(token, "XI8") == 0) {
        item_insert_append(context, at + append, CHUNK_TYPE_INT8);
    }
    if (strcmp(token, "Xi16") == 0) {
        item_insert_append(context, at + append, CHUNK_TYPE_UINT16);
    }
    if (strcmp(token, "XI16") == 0) {
        item_insert_append(context, at + append, CHUNK_TYPE_INT16);
    }
    if (strcmp(token, "Xi32") == 0) {
        item_insert_append(context, at + append, CHUNK_TYPE_UINT32);
    }
    if (strcmp(token, "XI32") == 0) {
        item_insert_append(context, at + append, CHUNK_TYPE_INT32);
    }
    if (strcmp(token, "Xi64") == 0) {
        item_insert_append(context, at + append, CHUNK_TYPE_UINT64);
    }
    if (strcmp(token, "XI64") == 0) {
        item_insert_append(context, at + append, CHUNK_TYPE_INT64);
    }
    if (strcmp(token, "Xf32") == 0) {
        item_insert_append(context, at + append, CHUNK_TYPE_FLOAT32);
    }
    if (strcmp(token, "Xf64") == 0) {
        item_insert_append(context, at + append, CHUNK_TYPE_FLOAT64);
    }
    if (strcmp(token, "Xs") == 0) {
        item_insert_append(context, at + append, CHUNK_TYPE_UTF8);
    }
    if (strcmp(token, "XR") == 0) {
        item_insert_append(context, at + append, CHUNK_TYPE_REF);
    }
    if (strcmp(token, "X[") == 0) {
        item_insert_append(context, at + append, CHUNK_TYPE_SET);
    }
    return reset_buffer(context, 1, CURSES_MODE_MOVE);
}

uint8_t handle_mode_cmdinput(c_context_t* context, int c) {
    uint8_t render = 0;
    switch (c) {
        case 0x20:
            render = interpret_command(context);
            break;
        case 0x0a:
            render = interpret_command(context);
            break;
        case KEY_BACKSPACE:
            if (context->cmd_buf_idx != 0) {
                context->cmd_buf_idx--;
            }
            context->cmd_buf[context->cmd_buf_idx] = '\0';
            render = 1;
            break;
        default:
            if ((c >= 0x21) && (c <= 0x7e)) {
                context->cmd_buf[context->cmd_buf_idx] = c;
                context->cmd_buf_idx++;
                render = 1;
            }
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
            case CURSES_MODE_CMDINPUT:
                render = handle_mode_cmdinput(context, c);
                break;
//            case CURSES_MODE_DATINPUT:
//                render = handle_mode_datinput(context, c);
//                break;
            default:
                break;
        }
        if (render) {
            clear();
            draw(context, 1, 1);
            refresh();
            render = 0;
        }
    }
}

void init_context(c_context_t* context) {
    memset(context, 0, sizeof(c_context_t));
    context->fd = -1;
    context->tabstop = 2;
    context->mode = CURSES_MODE_MOVE;
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
    init_pair(0x0f, COLOR_RED, COLOR_BLACK);
    init_pair(0x10, COLOR_YELLOW, COLOR_BLACK);

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
    init_pair(0x1f, COLOR_BLACK, COLOR_RED);
    init_pair(0x20, COLOR_YELLOW, COLOR_YELLOW);
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

/* Aria_lang/src/stdlib/terminal.c */
#include <stdio.h>
#include <stdint.h>

typedef uint64_t Value;
#define PTR_MASK 0x0000FFFFFFFFFFFFULL
#define TAG_INTEGER (0xFFF8000000000000ULL | 4ULL)
static inline Value box_int(int32_t i) { return TAG_INTEGER | (uint32_t)i; }
static inline int32_t unbox_int(Value v) { return (int32_t)(v & 0xFFFFFFFF); }

void* term_clear() {
    printf("\033[2J\033[H");
    return (void*)box_int(0);
}

void* term_move(void* x_tag, void* y_tag) {
    int x = unbox_int((Value)x_tag);
    int y = unbox_int((Value)y_tag);
    printf("\033[%d;%dH", y, x);
    return (void*)box_int(0);
}

void* term_color(void* fg_tag, void* bg_tag) {
    int fg = unbox_int((Value)fg_tag);
    int bg = unbox_int((Value)bg_tag);
    printf("\033[38;5;%dm\033[48;5;%dm", fg, bg);
    return (void*)box_int(0);
}

void* term_reset() {
    printf("\033[0m");
    return (void*)box_int(0);
}

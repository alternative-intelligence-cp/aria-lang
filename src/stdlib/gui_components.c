/* Aria_lang/src/stdlib/gui_components.c */
#include <X11/Xlib.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

// --- Imports ---
typedef struct {
    Display* dpy;
    Window win;
    int screen;
    Atom wm_delete_window;
    GC gc;
    XFontStruct* font;
} WinContext;

// We link against window.c
extern WinContext* get_win_context();
extern void* aria_alloc(size_t size);

typedef uint64_t Value;
#define TAG_INTEGER (0xFFF8000000000000ULL | 4ULL)
#define PTR_MASK    0x0000FFFFFFFFFFFFULL

static inline int32_t unbox_int(Value v) { return (int32_t)(v & 0xFFFFFFFF); }
static inline char* unbox_str(Value v) { return (char*)(v & PTR_MASK); }

// --- Styling Constants ---
#define COL_BG      0xEEEEEE
#define COL_BTN     0xDDDDDD
#define COL_BTN_H   0xCCCCCC
#define COL_TEXT    0x000000
#define COL_BORDER  0x888888
#define COL_GREEN   0x00AA00
#define COL_WHITE   0xFFFFFF

// --- Primitives ---

// Internal helper for drawing rectangles
void gui_draw_rect(int x, int y, int w, int h, int color) {
    WinContext* c = get_win_context();
    if (!c) return;
    
    // Fill
    XSetForeground(c->dpy, c->gc, color);
    XFillRectangle(c->dpy, c->win, c->gc, x, y, w, h);
    
    // Outline
    XSetForeground(c->dpy, c->gc, COL_BORDER);
    XDrawRectangle(c->dpy, c->win, c->gc, x, y, w, h);
}

void gui_draw_text(int x, int y, char* text) {
    WinContext* c = get_win_context();
    if (!c ||!text) return;
    XSetForeground(c->dpy, c->gc, COL_TEXT);
    // y in XDrawString is the baseline, so we offset simply
    XDrawString(c->dpy, c->win, c->gc, x, y, text, strlen(text));
}

// --- Widgets ---

// gui_btn(x, y, w, h, label, active)
// Draws a standard button. 'active' boolean determines styling (hover state).
void gui_btn(void* x_t, void* y_t, void* w_t, void* h_t, void* label_t, void* active_t) {
    int x = unbox_int((Value)x_t);
    int y = unbox_int((Value)y_t);
    int w = unbox_int((Value)w_t);
    int h = unbox_int((Value)h_t);
    char* label = unbox_str((Value)label_t);
    int active = unbox_int((Value)active_t); // 1 = hover/pressed

    // Draw Box
    gui_draw_rect(x, y, w, h, active? COL_BTN_H : COL_BTN);
    
    // Draw Centered Text
    int text_len = label? strlen(label) : 0;
    // Approximation: 6 pixels per char (fixed font)
    int tx = x + (w - (text_len * 6)) / 2; 
    int ty = y + (h / 2) + 4; // Vertical centering approximation
    gui_draw_text(tx, ty, label);
}

// gui_label(x, y, text)
// Draws static text.
void gui_label(void* x_t, void* y_t, void* text_t) {
    int x = unbox_int((Value)x_t);
    int y = unbox_int((Value)y_t);
    char* text = unbox_str((Value)text_t);
    gui_draw_text(x, y, text);
}

// gui_progress(x, y, w, h, percent)
// Draws a progress bar. Percent is 0-100.
void gui_progress(void* x_t, void* y_t, void* w_t, void* h_t, void* val_t) {
    int x = unbox_int((Value)x_t);
    int y = unbox_int((Value)y_t);
    int w = unbox_int((Value)w_t);
    int h = unbox_int((Value)h_t);
    
    double val = (double)unbox_int((Value)val_t) / 100.0;
    if (val > 1.0) val = 1.0;
    if (val < 0.0) val = 0.0;

    WinContext* c = get_win_context();
    if (!c) return;

    // Draw Background
    XSetForeground(c->dpy, c->gc, COL_WHITE);
    XFillRectangle(c->dpy, c->win, c->gc, x, y, w, h);
    XSetForeground(c->dpy, c->gc, COL_BORDER);
    XDrawRectangle(c->dpy, c->win, c->gc, x, y, w, h);

    // Draw Fill
    int fill_w = (int)(w * val);
    if (fill_w > 0) {
        XSetForeground(c->dpy, c->gc, COL_GREEN);
        // Inset by 1px so we don't overdraw border
        XFillRectangle(c->dpy, c->win, c->gc, x+1, y+1, fill_w-2, h-2);
    }
}

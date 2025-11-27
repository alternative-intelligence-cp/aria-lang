/* Aria_lang/src/stdlib/2d_drawing.c */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

extern void* aria_alloc(size_t size);

// --- Tagging Helpers ---
typedef uint64_t Value;
#define PTR_MASK 0x0000FFFFFFFFFFFFULL
#define TAG_OBJECT 0xFFF8000000000006ULL // Match object tag
static inline Value box_ptr(void* ptr, uint64_t tag) { return tag | (uintptr_t)ptr; }
static inline void* unbox_ptr(Value v) { return (void*)(v & PTR_MASK); }
static inline int32_t unbox_int(Value v) { return (int32_t)(v & 0xFFFFFFFF); }
static inline char* unbox_str(Value v) { return (char*)(v & PTR_MASK); }

typedef struct {
    int width;
    int height;
    unsigned char* pixels; 
} Canvas;

void* draw_new(void* w_tagged, void* h_tagged) {
    int w = unbox_int((Value)w_tagged);
    int h = unbox_int((Value)h_tagged);
    
    Canvas* c = (Canvas*)aria_alloc(sizeof(Canvas));
    c->width = w;
    c->height = h;
    c->pixels = (unsigned char*)aria_alloc(w * h * 3);
    memset(c->pixels, 0, w * h * 3);
    return (void*)box_ptr(c, TAG_OBJECT);
}

void draw_pixel(void* ptr_tagged, void* x_t, void* y_t, void* r_t, void* g_t, void* b_t) {
    Canvas* c = (Canvas*)unbox_ptr((Value)ptr_tagged);
    int x = unbox_int((Value)x_t);
    int y = unbox_int((Value)y_t);
    int r = unbox_int((Value)r_t);
    int g = unbox_int((Value)g_t);
    int b = unbox_int((Value)b_t);

    if (!c) return;
    if (x < 0 || x >= c->width || y < 0 || y >= c->height) return;
    
    int idx = (y * c->width + x) * 3;
    c->pixels[idx] = (unsigned char)r;
    c->pixels[idx+1] = (unsigned char)g;
    c->pixels[idx+2] = (unsigned char)b;
}

void draw_save(void* ptr_tagged, void* filename_tagged) {
    Canvas* c = (Canvas*)unbox_ptr((Value)ptr_tagged);
    char* filename = unbox_str((Value)filename_tagged);
    if (!c || !filename) return;
    
    FILE* f = fopen(filename, "wb");
    if (!f) return;
    
    fprintf(f, "P6\n%d %d\n255\n", c->width, c->height);
    fwrite(c->pixels, 1, c->width * c->height * 3, f);
    fclose(f);
}

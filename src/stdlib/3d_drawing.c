/* Aria_lang/src/stdlib/3d_drawing.c */
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <float.h>
#include <stdio.h>

extern void* aria_alloc(size_t size);

typedef uint64_t Value;
#define PTR_MASK 0x0000FFFFFFFFFFFFULL
#define TAG_OBJECT (0xFFF8000000000000ULL | 6ULL)

static inline Value box_ptr(void* ptr) { return TAG_OBJECT | (uintptr_t)ptr; }
static inline void* unbox_ptr(Value v) { return (void*)(v & PTR_MASK); }
static inline int32_t unbox_int(Value v) { return (int32_t)(v & 0xFFFFFFFF); }

// --- 3D Context ---
typedef struct {
    int width;
    int height;
    uint32_t* color_buffer; // ARGB
    float* z_buffer;        // Depth
} RenderContext;

// r3d_init(w, h)
void* r3d_init(void* w_t, void* h_t) {
    int w = unbox_int((Value)w_t);
    int h = unbox_int((Value)h_t);
    RenderContext* ctx = aria_alloc(sizeof(RenderContext));
    ctx->width = w;
    ctx->height = h;
    // Allocate buffers
    ctx->color_buffer = aria_alloc(w * h * sizeof(uint32_t));
    ctx->z_buffer = aria_alloc(w * h * sizeof(float));
    return (void*)box_ptr(ctx);
}

// r3d_clear(ctx, color)
void r3d_clear(void* ctx_t, void* color_t) {
    RenderContext* ctx = unbox_ptr((Value)ctx_t);
    uint32_t col = (uint32_t)unbox_int((Value)color_t);
    int count = ctx->width * ctx->height;
    for(int i=0; i<count; i++) {
        ctx->color_buffer[i] = col;
        ctx->z_buffer[i] = -FLT_MAX; // Initialize depth to far plane
    }
}

// Struct for internal vertex processing
typedef struct { float x, y, z; } Vec3;

// Edge Function: Returns positive if point is to the right of the edge
static inline float edge_func(Vec3 a, Vec3 b, Vec3 c) {
    return (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x);
}

// r3d_triangle(ctx, v1, v2, v3, color)
// vN are pointers to float arrays [x, y, z]
void r3d_triangle(void* ctx_t, void* v1_t, void* v2_t, void* v3_t, void* color_t) {
    RenderContext* ctx = unbox_ptr((Value)ctx_t);
    float* v1f = unbox_ptr((Value)v1_t);
    float* v2f = unbox_ptr((Value)v2_t);
    float* v3f = unbox_ptr((Value)v3_t);
    uint32_t color = (uint32_t)unbox_int((Value)color_t);

    Vec3 v0 = { v1f[0], v1f[1], v1f[2] };
    Vec3 v1 = { v2f[0], v2f[1], v2f[2] };
    Vec3 v2 = { v3f[0], v3f[1], v3f[2] };

    // 1. Perspective Projection
    // ScreenX = (x / -z) * scale + offset
    // We assume camera is at origin looking down -Z.
    float fov = 500.0f;
    
    // Backface Culling / Near Plane Clip Check
    if (v0.z >= -0.1f || v1.z >= -0.1f || v2.z >= -0.1f) {
        return; // Skip triangles too close to camera
    }

    v0.x = (v0.x / -v0.z) * fov + ctx->width/2.0f; 
    v0.y = (-v0.y / -v0.z) * fov + ctx->height/2.0f;
    v1.x = (v1.x / -v1.z) * fov + ctx->width/2.0f; 
    v1.y = (-v1.y / -v1.z) * fov + ctx->height/2.0f;
    v2.x = (v2.x / -v2.z) * fov + ctx->width/2.0f; 
    v2.y = (-v2.y / -v2.z) * fov + ctx->height/2.0f;

    // 2. Bounding Box Calculation
    int min_x = (int)fmin(v0.x, fmin(v1.x, v2.x));
    int min_y = (int)fmin(v0.y, fmin(v1.y, v2.y));
    int max_x = (int)fmax(v0.x, fmax(v1.x, v2.x));
    int max_y = (int)fmax(v0.y, fmax(v1.y, v2.y));

    // Screen Clipping
    min_x = fmax(0, min_x); min_y = fmax(0, min_y);
    max_x = fmin(ctx->width - 1, max_x); max_y = fmin(ctx->height - 1, max_y);

    float area = edge_func(v0, v1, v2);
    // If area is negative, it's a back-facing triangle (cull it)
    if (area <= 0) return;

    // 3. Rasterization Loop
    for (int y = min_y; y <= max_y; y++) {
        for (int x = min_x; x <= max_x; x++) {
            // Pixel Center
            Vec3 p = { x + 0.5f, y + 0.5f, 0 };
            
            // Calculate Barycentric Weights
            float w0 = edge_func(v1, v2, p);
            float w1 = edge_func(v2, v0, p);
            float w2 = edge_func(v0, v1, p);

            // If inside triangle
            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                w0 /= area; w1 /= area; w2 /= area;
                
                // Perspective Correct Z-Interpolation
                // 1/z = w0*(1/z0) + w1*(1/z1) + w2*(1/z2)
                float reciprocal_z = w0*(1.0f/v0.z) + w1*(1.0f/v1.z) + w2*(1.0f/v2.z);
                float z = 1.0f / reciprocal_z;
                
                // Depth Test
                int idx = y * ctx->width + x;
                // Note: We use > because z is negative (e.g. -5 is closer than -10)
                if (z > ctx->z_buffer[idx]) {
                    ctx->z_buffer[idx] = z;
                    ctx->color_buffer[idx] = color;
                }
            }
        }
    }
}

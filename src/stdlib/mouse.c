/* Aria_lang/src/stdlib/mouse.c */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <stdint.h>

extern void* aria_alloc(size_t size);

typedef uint64_t Value;
#define TAG_INTEGER (0xFFF8000000000000ULL | 4ULL)
static inline Value box_int(int32_t i) { return TAG_INTEGER | (uint32_t)i; }

static int mouse_fd = -1;

void* mouse_init() {
    // Try standard PS/2 mouse interface first, then event interface
    mouse_fd = open("/dev/input/mice", O_RDONLY | O_NONBLOCK);
    if (mouse_fd < 0) return box_int(0);
    return box_int(1);
}

// Returns a tagged integer packing:
// Bits 0-7:   Buttons (L=1, R=2, M=4)
// Bits 8-15:  Rel X (signed char cast to u8)
// Bits 16-23: Rel Y (signed char cast to u8)
void* mouse_poll() {
    if (mouse_fd < 0) return box_int(0);
    
    unsigned char data[3];
    int bytes = read(mouse_fd, data, 3);
    
    if (bytes > 0) {
        int left = (data[0] & 0x1) ? 1 : 0;
        int right = (data[0] & 0x2) ? 1 : 0;
        int middle = (data[0] & 0x4) ? 1 : 0;
        
        int rel_x = (signed char)data[1];
        int rel_y = (signed char)data[2];
        
        int32_t packed = (left | (right << 1) | (middle << 2)) |
                         ((rel_x & 0xFF) << 8) |
                         ((rel_y & 0xFF) << 16);
                         
        return box_int(packed);
    }
    return box_int(0); // No event
}

void mouse_close() {
    if (mouse_fd >= 0) close(mouse_fd);
    mouse_fd = -1;
}

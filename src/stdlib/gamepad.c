/* Aria_lang/src/stdlib/gamepad.c */
#include <fcntl.h>
#include <unistd.h>
#include <linux/joystick.h>
#include <stdint.h>

typedef uint64_t Value;
#define TAG_INTEGER (0xFFF8000000000000ULL | 4ULL)
static inline Value box_int(int32_t i) { return TAG_INTEGER | (uint32_t)i; }

static int fd = -1;

void* gp_init() {
    fd = open("/dev/input/js0", O_RDONLY | O_NONBLOCK);
    return (void*)box_int(fd);
}

// Returns: type | (number << 8) | (value << 16)
// Type: 1=Button, 2=Axis
void* gp_poll() {
    if (fd < 0) return (void*)box_int(0);
    struct js_event e;
    if (read(fd, &e, sizeof(e)) == sizeof(e)) {
        int type = e.type & ~JS_EVENT_INIT;
        int ret = 0;
        if (type == JS_EVENT_BUTTON) ret = 1;
        else if (type == JS_EVENT_AXIS) ret = 2;
        else return (void*)box_int(0);
        
        int32_t res = ret | (e.number << 8) | ((uint16_t)e.value << 16);
        return (void*)box_int(res);
    }
    return (void*)box_int(0);
}

void gp_close() {
    if (fd >= 0) close(fd);
    fd = -1;
}

/* Aria_lang/src/stdlib/clock.c */
#include <time.h>
#include <unistd.h>
#include <stdint.h>

typedef uint64_t Value;
#define QNAN_MASK       0x7FF8000000000000ULL
#define SIGN_BIT        0x8000000000000000ULL
#define TAG_BASE        (QNAN_MASK | SIGN_BIT)
#define TAG_INTEGER     (TAG_BASE | 4ULL)

static inline Value box_int(int32_t i) { return TAG_INTEGER | (uint32_t)i; }
static inline int32_t unbox_int(Value v) { return (int32_t)(v & 0xFFFFFFFF); }

// Returns current unix timestamp (seconds) as tagged int
void* clock_now() {
    return (void*)box_int((int32_t)time(NULL));
}

// Sleep for N seconds
void clock_sleep(void* sec_tagged) {
    int s = unbox_int((Value)sec_tagged);
    if (s > 0) sleep(s);
}

// Sleep for N milliseconds
void clock_msleep(void* ms_tagged) {
    int ms = unbox_int((Value)ms_tagged);
    if (ms > 0) usleep(ms * 1000);
}

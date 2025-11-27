/* Aria_lang/src/stdlib/window.c */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// --- Runtime Imports ---
extern void* aria_alloc(size_t size);
extern void* aria_alloc_object();
extern void* aria_obj_set(void* o, char* k, void* v);

// --- Tagging System ---
typedef uint64_t Value;
#define TAG_INTEGER (0xFFF8000000000000ULL | 4ULL)
#define TAG_STRING  (0xFFF8000000000000ULL | 5ULL)
#define TAG_OBJECT  (0xFFF8000000000000ULL | 6ULL)
#define PTR_MASK    0x0000FFFFFFFFFFFFULL

static inline Value box_int(int32_t i) { return TAG_INTEGER | (uint32_t)i; }
static inline Value box_ptr(void* ptr, uint64_t tag) { return tag | (uintptr_t)ptr; }
static inline Value box_str(char* s) { return TAG_STRING | (uintptr_t)s; }

// --- Shared Context Definition ---
// This context is shared with gui_components.c
typedef struct {
    Display* dpy;
    Window win;
    int screen;
    Atom wm_delete_window;
    GC gc;
    XFontStruct* font;
} WinContext;

// Global Singleton for the Main Window
static WinContext ctx;
static int initialized = 0;

// Export for usage in gui_components.c
WinContext* get_win_context() {
    return initialized? &ctx : NULL;
}

// --- Window Management API ---

// win_create(width, height, title) -> Success(1) | Failure(0)
void* win_create(void* w_t, void* h_t, void* title_t) {
    int w = (int)((Value)w_t & 0xFFFFFFFF);
    int h = (int)((Value)h_t & 0xFFFFFFFF);
    char* title = (char*)((Value)title_t & PTR_MASK);
    if (!title) title = "Aria Application";

    if (!initialized) {
        ctx.dpy = XOpenDisplay(NULL);
        if (!ctx.dpy) {
            fprintf(stderr, "GUI Error: Cannot open X Display.\n");
            return (void*)box_int(0);
        }
        ctx.screen = DefaultScreen(ctx.dpy);
        initialized = 1;
    }

    // Create the Window
    // We use DefaultRootWindow as parent to place it on the desktop.
    ctx.win = XCreateSimpleWindow(ctx.dpy, RootWindow(ctx.dpy, ctx.screen), 
                                  10, 10, w, h, 1,
                                  BlackPixel(ctx.dpy, ctx.screen), 
                                  WhitePixel(ctx.dpy, ctx.screen));

    // Set Window Title
    XStoreName(ctx.dpy, ctx.win, title);
    
    // Crucial: Select Input Masks
    // Without this, the X Server will discard events targetting this window.
    XSelectInput(ctx.dpy, ctx.win, 
        ExposureMask | // Window needs redrawing
        KeyPressMask | // Keyboard input
        ButtonPressMask | // Mouse click
        ButtonReleaseMask | // Mouse release
        PointerMotionMask | // Mouse movement
        StructureNotifyMask     // Resize events
    );
    
    // Handle the "X" close button in the window chrome
    ctx.wm_delete_window = XInternAtom(ctx.dpy, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(ctx.dpy, ctx.win, &ctx.wm_delete_window, 1);
    
    // Setup Graphics Context (GC) for drawing
    ctx.gc = XCreateGC(ctx.dpy, ctx.win, 0, NULL);
    
    // Load a default font
    ctx.font = XLoadQueryFont(ctx.dpy, "fixed");
    if (ctx.font) XSetFont(ctx.dpy, ctx.gc, ctx.font->fid);

    // Show the window
    XMapWindow(ctx.dpy, ctx.win);
    
    // Flush commands to server immediately
    XFlush(ctx.dpy);

    return (void*)box_int(1);
}

// win_next_event() -> EventObject | Null
// Polls the X Server for the next event. 
// Returns a tagged Object with fields: { type: "...", key:..., x:..., y:... }
void* win_next_event() {
    if (!initialized) return NULL;
    
    // Check if events are pending to avoid blocking the Aria VM if it wants to do other work
    if (XPending(ctx.dpy) == 0) return NULL;

    XEvent ev;
    XNextEvent(ctx.dpy, &ev); // Fetch event

    void* event_obj = aria_alloc_object();

    if (ev.type == Expose) {
        // Redraw request
        aria_obj_set(event_obj, "type", (void*)box_str("expose"));
    }
    else if (ev.type == ButtonPress) {
        aria_obj_set(event_obj, "type", (void*)box_str("mousedown"));
        aria_obj_set(event_obj, "x", (void*)box_int(ev.xbutton.x));
        aria_obj_set(event_obj, "y", (void*)box_int(ev.xbutton.y));
        aria_obj_set(event_obj, "btn", (void*)box_int(ev.xbutton.button));
    }
    else if (ev.type == ButtonRelease) {
        aria_obj_set(event_obj, "type", (void*)box_str("mouseup"));
        aria_obj_set(event_obj, "x", (void*)box_int(ev.xbutton.x));
        aria_obj_set(event_obj, "y", (void*)box_int(ev.xbutton.y));
    }
    else if (ev.type == MotionNotify) {
        aria_obj_set(event_obj, "type", (void*)box_str("mousemove"));
        aria_obj_set(event_obj, "x", (void*)box_int(ev.xmotion.x));
        aria_obj_set(event_obj, "y", (void*)box_int(ev.xmotion.y));
    }
    else if (ev.type == KeyPress) {
        aria_obj_set(event_obj, "type", (void*)box_str("keydown"));
        KeySym keysym = XLookupKeysym(&ev.xkey, 0);
        aria_obj_set(event_obj, "keycode", (void*)box_int((int)keysym));
        
        // Translate KeySym to printable char if possible
        char buf;
        XLookupString(&ev.xkey, buf, 32, NULL, NULL);
        if (strlen(buf) > 0) {
            char* s = (char*)aria_alloc(strlen(buf)+1);
            strcpy(s, buf);
            aria_obj_set(event_obj, "char", (void*)box_str(s));
        }
    }
    else if (ev.type == ClientMessage) {
        if ((Atom)ev.xclient.data.l == ctx.wm_delete_window) {
            aria_obj_set(event_obj, "type", (void*)box_str("quit"));
        }
    }

    return (void*)box_ptr(event_obj, TAG_OBJECT);
}

// win_close()
// Clean up X resources.
void win_close() {
    if (initialized) {
        if (ctx.font) XFreeFont(ctx.dpy, ctx.font);
        XFreeGC(ctx.dpy, ctx.gc);
        XDestroyWindow(ctx.dpy, ctx.win);
        XCloseDisplay(ctx.dpy);
        initialized = 0;
    }
}

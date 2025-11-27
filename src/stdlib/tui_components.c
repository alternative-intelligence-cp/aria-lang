/* Aria_lang/src/stdlib/tui_components.c */
#include <stdio.h>
#include <stdint.h>

typedef uint64_t Value;

void tui_box(void* x_t, void* y_t, void* w_t, void* h_t) {
    int x = (int)((Value)x_t & 0xFFFFFFFF);
    int y = (int)((Value)y_t & 0xFFFFFFFF);
    int w = (int)((Value)w_t & 0xFFFFFFFF);
    int h = (int)((Value)h_t & 0xFFFFFFFF);
    
    printf("\033[%d;%dH", y, x);
    // Draw top border
    for(int i=0; i<w; i++) printf("#");
    // Draw sides
    for(int i=1; i<h-1; i++) {
        printf("\033[%d;%dH#", y+i, x);
        printf("\033[%d;%dH#", y+i, x+w-1);
    }
    // Draw bottom
    printf("\033[%d;%dH", y+h-1, x);
    for(int i=0; i<w; i++) printf("#");
}

//add further terminal user interface features for the full bells and whistles experience

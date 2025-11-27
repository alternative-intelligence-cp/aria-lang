/* Aria_lang/utils/bin2c.c */
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: bin2c <input_file> <array_name>\n");
        return 1;
    }

    FILE* f = fopen(argv[1], "rb");
    if (!f) {
        fprintf(stderr, "Could not open %s\n", argv[1]);
        return 1;
    }

    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);

    printf("const unsigned int %s_len = %ld;\n", argv[2], len);
    printf("const unsigned char %s[] = {", argv[2]);

    unsigned char buf[1024];
    size_t n;
    int first = 1;
    while ((n = fread(buf, 1, sizeof(buf), f)) > 0) {
        for (size_t i = 0; i < n; i++) {
            if (!first) printf(",");
            printf("0x%02x", buf[i]);
            first = 0;
        }
    }
    printf("};\n");

    fclose(f);
    return 0;
}

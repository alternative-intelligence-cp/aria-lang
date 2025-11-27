/* Aria_lang/src/stdlib/fetch.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdint.h>

extern void* aria_alloc(size_t size);

typedef uint64_t Value;
#define TAG_STRING 0xFFF8000000000005ULL
static inline Value box_ptr(void* ptr, uint64_t tag) { return tag | (uintptr_t)ptr; }
static inline char* unbox_str(Value v) { return (char*)(v & 0x0000FFFFFFFFFFFFULL); }

static void parse_url(char* url, char* host, int* port, char* path) {
    *port = 80;
    strcpy(path, "/");
    char* p = url;
    if (strncmp(p, "http://", 7) == 0) p += 7;
    int h_len = 0;
    while (*p && *p != ':' && *p != '/') {
        if (h_len < 255) host[h_len++] = *p;
        p++;
    }
    host[h_len] = '\0';
    if (*p == ':') { p++; *port = atoi(p); while (*p && *p != '/') p++; }
    if (*p == '/') { strncpy(path, p, 1023); path[1023] = '\0'; }
}

// Returns TAGGED string
void* fetch_get(void* url_tagged) {
    char* url = unbox_str((Value)url_tagged);
    if (!url) return NULL;
    
    char host[256]; int port; char path[1024];
    parse_url(url, host, &port, path);
    
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return NULL;
    
    struct hostent* he = gethostbyname(host);
    if (!he) { close(sock); return NULL; }
    
    struct sockaddr_in server;
    memcpy(&server.sin_addr, he->h_addr_list, he->h_length);
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    
    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) { close(sock); return NULL; }
    
    char request[2048];
    snprintf(request, sizeof(request), "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", path, host);
    if (send(sock, request, strlen(request), 0) < 0) { close(sock); return NULL; }
    
    size_t cap = 4096; size_t len = 0; char* buf = malloc(cap);
    if (!buf) { close(sock); return NULL; }
    
    int received;
    while ((received = recv(sock, buf + len, cap - len - 1, 0)) > 0) {
        len += received;
        if (len + 1024 >= cap) {
            cap *= 2;
            char* new_buf = realloc(buf, cap);
            if (!new_buf) { free(buf); close(sock); return NULL; }
            buf = new_buf;
        }
    }
    buf[len] = '\0';
    close(sock);
    
    char* body = strstr(buf, "\r\n\r\n");
    char* result = NULL;
    if (body) {
        body += 4;
        size_t body_len = strlen(body);
        result = (char*)aria_alloc(body_len + 1);
        memcpy(result, body, body_len + 1);
    } else {
        result = (char*)aria_alloc(len + 1);
        memcpy(result, buf, len + 1);
    }
    free(buf);
    return (void*)box_ptr(result, TAG_STRING);
}

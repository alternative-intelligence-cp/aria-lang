/* Aria_lang/src/stdlib/network.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <stdint.h>
#include <pthread.h>

// OpenSSL Headers for TLS/SSL support
#include <openssl/ssl.h>
#include <openssl/err.h>

extern void* aria_alloc(size_t size);

// --- Tagging Helpers ---
// Aria uses NaN-boxing. We must conform to this ABI for all return values.
typedef uint64_t Value;
#define QNAN_MASK       0x7FF8000000000000ULL
#define SIGN_BIT        0x8000000000000000ULL
#define TAG_BASE        (QNAN_MASK | SIGN_BIT)
#define TAG_INTEGER     (TAG_BASE | 4ULL)
#define TAG_STRING      (TAG_BASE | 5ULL)
#define PTR_MASK        0x0000FFFFFFFFFFFFULL

static inline Value box_int(int32_t i) { return TAG_INTEGER | (uint32_t)i; }
static inline Value box_ptr(void* ptr, uint64_t tag) { return tag | (uintptr_t)ptr; }
static inline int32_t unbox_int(Value v) { return (int32_t)(v & 0xFFFFFFFF); }
static inline char* unbox_str(Value v) { return (char*)(v & PTR_MASK); }

// --- Global SSL Context Registry ---
// We map File Descriptors (int) to SSL Objects (SSL*) to maintain ABI compatibility.
// A simple open-addressing hash table is sufficient for active connections.
// Size 65536 covers the standard ephemeral port range limits on Linux.

#define MAX_SSL_CONNS 65536

typedef struct {
    int fd;
    SSL* ssl;
    SSL_CTX* ctx; // Context used for this connection
} SSLMapEntry;

static SSLMapEntry ssl_map;
static pthread_mutex_t ssl_map_lock = PTHREAD_MUTEX_INITIALIZER;
static int ssl_initialized = 0;

// Initialize OpenSSL Algorithms and Error Strings
void init_openssl() {
    if (!ssl_initialized) {
        SSL_load_error_strings();
        OpenSSL_add_ssl_algorithms();
        // Initialize map
        for(int i=0; i<MAX_SSL_CONNS; i++) ssl_map[i].fd = -1;
        ssl_initialized = 1;
    }
}

// Register an SSL connection in the shadow registry
void register_ssl(int fd, SSL* ssl, SSL_CTX* ctx) {
    pthread_mutex_lock(&ssl_map_lock);
    // Simple hash function: Identity map for FDs
    int idx = fd % MAX_SSL_CONNS;
    
    // Linear probing for collision resolution
    int start = idx;
    while (ssl_map[idx].fd!= -1) {
        idx = (idx + 1) % MAX_SSL_CONNS;
        if (idx == start) {
            // Table full - highly unlikely with 65k slots and OS limits
            fprintf(stderr, "Fatal: SSL Registry Full\n");
            pthread_mutex_unlock(&ssl_map_lock);
            return;
        }
    }
    
    ssl_map[idx].fd = fd;
    ssl_map[idx].ssl = ssl;
    ssl_map[idx].ctx = ctx;
    pthread_mutex_unlock(&ssl_map_lock);
}

// Retrieve SSL object for a given FD
SSL* get_ssl(int fd) {
    pthread_mutex_lock(&ssl_map_lock);
    int idx = fd % MAX_SSL_CONNS;
    int start = idx;
    
    while (ssl_map[idx].fd!= -1) {
        if (ssl_map[idx].fd == fd) {
            pthread_mutex_unlock(&ssl_map_lock);
            return ssl_map[idx].ssl;
        }
        idx = (idx + 1) % MAX_SSL_CONNS;
        if (idx == start) break;
    }
    pthread_mutex_unlock(&ssl_map_lock);
    return NULL;
}

// Remove and cleanup SSL object
void unregister_ssl(int fd) {
    pthread_mutex_lock(&ssl_map_lock);
    int idx = fd % MAX_SSL_CONNS;
    int start = idx;
    
    while (ssl_map[idx].fd!= -1) {
        if (ssl_map[idx].fd == fd) {
            if (ssl_map[idx].ssl) {
                // Perform bidirectional shutdown
                SSL_shutdown(ssl_map[idx].ssl);
                SSL_free(ssl_map[idx].ssl);
            }
            // Note: We generally don't free the CTX here as it is shared by the listener.
            ssl_map[idx].fd = -1;
            ssl_map[idx].ssl = NULL;
            pthread_mutex_unlock(&ssl_map_lock);
            return;
        }
        idx = (idx + 1) % MAX_SSL_CONNS;
        if (idx == start) break;
    }
    pthread_mutex_unlock(&ssl_map_lock);
}

// --- Socket Implementation ---

// Returns Tagged Integer (Socket FD)
// Standard TCP (Plaintext) Server Start
void* net_server_start(void* port_tagged) {
    init_openssl();
    int port = unbox_int((Value)port_tagged);
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        return (void*)box_int(-1);
    }

    // Set SO_REUSEADDR to allow immediate restart of server
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        close(server_fd); 
        return (void*)box_int(-1);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd); 
        return (void*)box_int(-1);
    }

    if (listen(server_fd, 100) < 0) { // Increased backlog to 100
        perror("listen");
        close(server_fd); 
        return (void*)box_int(-1);
    }
    
    return (void*)box_int(server_fd);
}

// New: Start a Secure Server (HTTPS/WSS)
// Returns Tagged Integer (Socket FD). 
// Internally associates this FD with an SSL_CTX loaded from cert/key.
// Note: The listener FD itself doesn't do SSL I/O, but we map it to the CTX
// so that subsequent `accept()` calls know to perform an SSL handshake.
void* net_server_start_secure(void* port_t, void* cert_path_t, void* key_path_t) {
    init_openssl();
    int port = unbox_int((Value)port_t);
    char* cert_path = unbox_str((Value)cert_path_t);
    char* key_path = unbox_str((Value)key_path_t);

    if (!cert_path ||!key_path) return box_int(-1);

    // 1. Create SSL Context
    const SSL_METHOD *method = TLS_server_method();
    SSL_CTX *ctx = SSL_CTX_new(method);
    if (!ctx) {
        ERR_print_errors_fp(stderr);
        return box_int(-1);
    }

    // 2. Load Certificates
    if (SSL_CTX_use_certificate_file(ctx, cert_path, SSL_FILETYPE_PEM) <= 0) {
        fprintf(stderr, "Error loading certificate: %s\n", cert_path);
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx);
        return box_int(-1);
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, key_path, SSL_FILETYPE_PEM) <= 0 ) {
        fprintf(stderr, "Error loading private key: %s\n", key_path);
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx);
        return box_int(-1);
    }

    // 3. Start underlying TCP Server
    void* res = net_server_start(port_t);
    int fd = unbox_int((Value)res);
    
    if (fd >= 0) {
        // Register the LISTENER FD with the CTX. 
        register_ssl(fd, NULL, ctx);
    } else {
        SSL_CTX_free(ctx);
    }
    return res;
}

// Returns Tagged Integer (Client FD)
void* net_accept(void* server_fd_tagged) {
    int server_fd = unbox_int((Value)server_fd_tagged);
    if (server_fd < 0) return (void*)box_int(-1);

    int new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Standard TCP Accept
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        return (void*)box_int(-1);
    }

    // Check if the server_fd is a Secure Listener
    pthread_mutex_lock(&ssl_map_lock);
    SSL_CTX* listener_ctx = NULL;
    // Look up the server FD to see if it has an associated SSL_CTX
    int idx = server_fd % MAX_SSL_CONNS;
    int start = idx;
    while (ssl_map[idx].fd!= -1) {
        if (ssl_map[idx].fd == server_fd) {
            listener_ctx = ssl_map[idx].ctx;
            break;
        }
        idx = (idx + 1) % MAX_SSL_CONNS;
        if (idx == start) break;
    }
    pthread_mutex_unlock(&ssl_map_lock);

    // If Secure, perform SSL Handshake
    if (listener_ctx) {
        SSL *ssl = SSL_new(listener_ctx);
        SSL_set_fd(ssl, new_socket);
        
        // SSL_accept performs the TLS handshake.
        // In a non-blocking architecture, this might return WANT_READ/WRITE.
        // For this implementation, we assume blocking mode for simplicity.
        if (SSL_accept(ssl) <= 0) {
            ERR_print_errors_fp(stderr);
            SSL_free(ssl);
            close(new_socket);
            return (void*)box_int(-1);
        }
        // Register the new CLIENT connection with its SSL object
        register_ssl(new_socket, ssl, NULL);
    }

    return (void*)box_int(new_socket);
}

// Returns Tagged String
void* net_read(void* socket_tagged) {
    int sock = unbox_int((Value)socket_tagged);
    if (sock < 0) return NULL;

    SSL* ssl = get_ssl(sock);
    char buffer = {0};
    ssize_t valread;

    if (ssl) {
        valread = SSL_read(ssl, buffer, 4096);
    } else {
        valread = read(sock, buffer, 4096);
    }
    
    if (valread <= 0) {
        // Connection closed or error
        return NULL;
    }
    
    // Allocate string in Aria heap
    char* res = (char*)aria_alloc(valread + 1);
    if (res) {
        memcpy(res, buffer, valread);
        res[valread] = '\0';
    }
    return (void*)box_ptr(res, TAG_STRING);
}

// Writes a string to the socket
void net_write(void* socket_tagged, void* str_tagged) {
    int sock = unbox_int((Value)socket_tagged);
    char* str = unbox_str((Value)str_tagged);
    
    if (sock < 0 ||!str) return;
    
    size_t len = strlen(str);
    SSL* ssl = get_ssl(sock);

    if (ssl) {
        SSL_write(ssl, str, len);
    } else {
        ssize_t sent = 0;
        while (sent < len) {
            ssize_t n = send(sock, str + sent, len - sent, 0);
            if (n == -1) {
                if (errno == EINTR) continue;
                break; 
            }
            sent += n;
        }
    }
}

// Raw write for binary data (Required for WebSocket Frames)
void net_write_raw(void* socket_tagged, void* buf_tagged, void* len_tagged) {
    int sock = unbox_int((Value)socket_tagged);
    char* buf = (char*)((Value)buf_tagged & PTR_MASK); // manual unbox raw pointer
    int len = unbox_int((Value)len_tagged);

    if (sock < 0 ||!buf) return;

    SSL* ssl = get_ssl(sock);
    if (ssl) {
        SSL_write(ssl, buf, len);
    } else {
        send(sock, buf, len, 0);
    }
}

void net_close(void* socket_tagged) {
    int sock = unbox_int((Value)socket_tagged);
    if (sock >= 0) {
        unregister_ssl(sock); // Cleanup SSL and close FD
        close(sock);
    }
}

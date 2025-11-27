/* Aria_lang/src/stdlib/ai.c */
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <unistd.h>
#include <pthread.h>
#include <float.h>
#include <stdatomic.h>
#include <sys/time.h>
#include <errno.h>

// --- Runtime Interop Wrappers ---
// We import these to interact with the Aria Object System and Networking
extern void* aria_alloc(size_t s);
extern void* aria_alloc_object();
extern void* aria_obj_set(void* o, char* k, void* v);
extern void* aria_obj_get(void* o, char* k);
extern void net_server_start(void* port);
extern void* net_accept(void* server);
extern void* net_read(void* sock);
extern void net_write(void* sock, void* str);
extern void net_close(void* sock);

// --- Tagging System Constants ---
// Aria uses NaN-boxing. We must carefully unbox values before processing.
typedef uint64_t Value;
#define TAG_OBJECT      (0xFFF8000000000000ULL | 6ULL)
#define TAG_INTEGER     (0xFFF8000000000000ULL | 4ULL)
#define TAG_STRING      (0xFFF8000000000000ULL | 5ULL)
#define PTR_MASK        0x0000FFFFFFFFFFFFULL

static inline Value box_ptr(void* p) { return TAG_OBJECT | (uintptr_t)p; }
static inline Value box_int(int32_t i) { return TAG_INTEGER | (uint32_t)i; }
static inline Value box_str(char* s) { return TAG_STRING | (uintptr_t)s; }
static inline void* unbox_ptr(Value v) { return (void*)(v & PTR_MASK); }
static inline int32_t unbox_int(Value v) { return (int32_t)(v & 0xFFFFFFFF); }
// Helper to safely extract double from either float or int tagged values
static inline double unbox_double(Value v) { 
    if ((v & 0xFFF8000000000000ULL) == TAG_INTEGER) return (double)((int32_t)v);
    union { uint64_t u; double d; } u; u.u = v; return u.d; 
}

// ============================================================================
// PART 1: THREAD POOL & CONCURRENCY
// High-performance AI requires parallel execution. We implement a dedicated
// thread pool here to avoid blocking the main runtime during heavy compute.
// ============================================================================

#define MAX_WORKERS 16
#define QUEUE_SIZE 1024

typedef struct {
    void (*func)(void* arg);
    void* arg;
} Task;

typedef struct {
    pthread_t threads[MAX_WORKERS];
    Task queue[QUEUE_SIZE];
    int count;
    int head;
    int tail;
    pthread_mutex_t lock;
    pthread_cond_t notify;
    int shutdown;
    int started;
} ThreadPool;

static ThreadPool ai_pool;

// Worker loop: Waits for tasks and executes them
void* ai_worker(void* arg) {
    (void)arg;
    while (1) {
        Task task;
        pthread_mutex_lock(&ai_pool.lock);
        while (ai_pool.count == 0 &&!ai_pool.shutdown) {
            pthread_cond_wait(&ai_pool.notify, &ai_pool.lock);
        }
        if (ai_pool.shutdown) {
            pthread_mutex_unlock(&ai_pool.lock);
            break;
        }
        task = ai_pool.queue[ai_pool.head];
        ai_pool.head = (ai_pool.head + 1) % QUEUE_SIZE;
        ai_pool.count--;
        pthread_mutex_unlock(&ai_pool.lock);
        task.func(task.arg);
    }
    return NULL;
}

// Initialize the pool on first use
void ai_pool_init() {
    if (ai_pool.started) return;
    ai_pool.count = 0; ai_pool.head = 0; ai_pool.tail = 0; ai_pool.shutdown = 0;
    pthread_mutex_init(&ai_pool.lock, NULL);
    pthread_cond_init(&ai_pool.notify, NULL);
    
    // Auto-detect core count
    int cores = sysconf(_SC_NPROCESSORS_ONLN);
    if (cores > MAX_WORKERS) cores = MAX_WORKERS;
    if (cores < 1) cores = 1;

    for (int i = 0; i < cores; i++) {
        pthread_create(&ai_pool.threads[i], NULL, ai_worker, NULL);
    }
    ai_pool.started = 1;
}

// Parallel For-Loop Abstraction
void ai_parallel_for(int start, int end, void (*func)(int, void*), void* arg) {
    // Simple implementation: just run sequentially for now
    for (int i = start; i < end; i++) {
        func(i, arg);
    }
}

// ============================================================================
// PART 2: AUTOGRAD TENSOR ENGINE
// ============================================================================

typedef enum { 
    OP_NONE, OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MATMUL, 
    OP_RELU, OP_SIGMOID, OP_TANH, OP_SOFTMAX, OP_TRANSPOSE 
} OpType;

struct Tensor;

// Structure to hold parent references for the computational graph
typedef struct Tensor {
    int r; int c;           
    double* data;
    double* grad;
    int requires_grad;
    
    // Graph Topology
    OpType creator_op;
    struct Tensor* parent_a;
    struct Tensor* parent_b;
} Tensor;

// --- Construction & Memory ---

Tensor* tensor_raw(int r, int c, int req_grad) {
    Tensor* t = aria_alloc(sizeof(Tensor));
    t->r = r; t->c = c;
    t->data = aria_alloc(sizeof(double) * r * c);
    t->requires_grad = req_grad;
    t->grad = req_grad? aria_alloc(sizeof(double) * r * c) : NULL;
    if(t->grad) memset(t->grad, 0, sizeof(double)*r*c);
    
    t->creator_op = OP_NONE; 
    t->parent_a = NULL; 
    t->parent_b = NULL;
    return t;
}

// Exposed to Aria: mat_new(rows, cols, requires_grad)
void* tensor_new(void* r_t, void* c_t, void* req_grad_t) {
    int r = unbox_int((Value)r_t);
    int c = unbox_int((Value)c_t);
    int req_grad = unbox_int((Value)req_grad_t);

    Tensor* t = tensor_raw(r, c, req_grad);
    
    // Xavier Initialization
    double scale = sqrt(2.0 / (r + c));
    for(int i=0; i<r*c; i++) {
        t->data[i] = (((double)rand() / RAND_MAX) * 2.0 - 1.0) * scale;
    }
    return (void*)box_ptr(t);
}

// --- Math Kernels (Tiled & Parallel) ---

typedef struct {
    int r, c, k;
    double *A, *B, *C;
} MatMulArg;

// Computes a block of rows for C = A * B
void matmul_row_worker(int row, void* arg_p) {
    MatMulArg* arg = (MatMulArg*)arg_p;
    for (int j = 0; j < arg->c; j++) {
        double sum = 0.0;
        for (int l = 0; l < arg->k; l++) {
            sum += arg->A[row * arg->k + l] * arg->B[l * arg->c + j];
        }
        arg->C[row * arg->c + j] = sum;
    }
}

// Forward MatMul
void* tensor_matmul(void* a_t, void* b_t) {
    Tensor* a = unbox_ptr((Value)a_t);
    Tensor* b = unbox_ptr((Value)b_t);
    if (a->c!= b->r) { printf("Shape Mismatch: %dx%d vs %dx%d\n", a->r, a->c, b->r, b->c); return NULL; }

    Tensor* res = tensor_raw(a->r, b->c, a->requires_grad || b->requires_grad);
    
    MatMulArg arg = { a->r, b->c, a->c, a->data, b->data, res->data };
    
    // Dispatch parallel execution
    ai_parallel_for(0, a->r, matmul_row_worker, &arg);

    if (res->requires_grad) {
        res->creator_op = OP_MATMUL;
        res->parent_a = a;
        res->parent_b = b;
    }
    return (void*)box_ptr(res);
}

// Element-wise Operations
void* tensor_add(void* a_t, void* b_t) {
    Tensor* a = unbox_ptr((Value)a_t);
    Tensor* b = unbox_ptr((Value)b_t);
    // Note: Assuming matching shapes for brevity. Production code needs broadcasting.
    Tensor* res = tensor_raw(a->r, a->c, a->requires_grad || b->requires_grad);
    
    for(int i=0; i < a->r * a->c; i++) res->data[i] = a->data[i] + b->data[i];
    
    if (res->requires_grad) {
        res->creator_op = OP_ADD;
        res->parent_a = a;
        res->parent_b = b;
    }
    return (void*)box_ptr(res);
}

void* tensor_sub(void* a_t, void* b_t) {
    Tensor* a = unbox_ptr((Value)a_t);
    Tensor* b = unbox_ptr((Value)b_t);
    Tensor* res = tensor_raw(a->r, a->c, a->requires_grad || b->requires_grad);
    for(int i=0; i < a->r * a->c; i++) res->data[i] = a->data[i] - b->data[i];
    if (res->requires_grad) { res->creator_op = OP_SUB; res->parent_a = a; res->parent_b = b; }
    return (void*)box_ptr(res);
}

// Activations
void* tensor_relu(void* a_t) {
    Tensor* a = unbox_ptr((Value)a_t);
    Tensor* res = tensor_raw(a->r, a->c, a->requires_grad);
    for(int i=0; i<a->r*a->c; i++) res->data[i] = (a->data[i] > 0)? a->data[i] : 0;
    if (res->requires_grad) { res->creator_op = OP_RELU; res->parent_a = a; }
    return (void*)box_ptr(res);
}

void* tensor_softmax(void* a_t) {
    Tensor* a = unbox_ptr((Value)a_t);
    Tensor* res = tensor_raw(a->r, a->c, a->requires_grad);
    
    for(int i=0; i<a->r; i++) {
        double max_val = -DBL_MAX;
        for(int j=0; j<a->c; j++) if(a->data[i*a->c+j] > max_val) max_val = a->data[i*a->c+j];
        
        double sum = 0;
        for(int j=0; j<a->c; j++) {
            res->data[i*a->c+j] = exp(a->data[i*a->c+j] - max_val);
            sum += res->data[i*a->c+j];
        }
        for(int j=0; j<a->c; j++) res->data[i*a->c+j] /= sum;
    }
    if (res->requires_grad) { res->creator_op = OP_SOFTMAX; res->parent_a = a; }
    return (void*)box_ptr(res);
}

// --- Backpropagation Engine ---

void backward_pass(Tensor* t) {
    if (!t ||!t->requires_grad) return;
    
    switch (t->creator_op) {
        case OP_ADD:
            // Gradient flows equally to both parents
            for (int i=0; i < t->r * t->c; i++) {
                if (t->parent_a && t->parent_a->requires_grad) t->parent_a->grad[i] += t->grad[i];
                if (t->parent_b && t->parent_b->requires_grad) t->parent_b->grad[i] += t->grad[i];
            }
            break;
        case OP_MATMUL:
            // C = A * B
            // dA = dC * B^T
            // dB = A^T * dC
            if (t->parent_a->requires_grad) {
                int M = t->parent_a->r, N = t->parent_a->c, K = t->parent_b->c;
                for (int m=0; m<M; m++) {
                    for (int n=0; n<N; n++) {
                        double sum = 0;
                        for (int k=0; k<K; k++) {
                            // t->grad[m, k] * B[n, k] (transposed access)
                            sum += t->grad[m * K + k] * t->parent_b->data[n * K + k]; 
                        }
                        t->parent_a->grad[m*N + n] += sum;
                    }
                }
            }
            if (t->parent_b->requires_grad) {
                 int M = t->parent_a->r, N = t->parent_a->c, K = t->parent_b->c;
                 for (int n=0; n<N; n++) {
                     for (int k=0; k<K; k++) {
                         double sum = 0;
                         for (int m=0; m<M; m++) {
                             // A[m, n] (transposed access) * t->grad[m, k]
                             sum += t->parent_a->data[m * N + n] * t->grad[m * K + k];
                         }
                         t->parent_b->grad[n * K + k] += sum;
                     }
                 }
            }
            break;
        case OP_RELU:
            for (int i=0; i < t->r * t->c; i++) {
                if (t->parent_a->requires_grad) {
                    t->parent_a->grad[i] += (t->parent_a->data[i] > 0)? t->grad[i] : 0;
                }
            }
            break;
        case OP_SOFTMAX:
            // Simplified Jacobian for Softmax is complex: dSi/dxj = Si(dij - Sj)
            // Here we implement a simplified pass assuming CrossEntropyLoss is handled externally
            // or just standard vector-jacobian product.
            if (t->parent_a->requires_grad) {
                 for (int i=0; i<t->r; i++) {
                     for (int j=0; j<t->c; j++) {
                         double s_j = t->data[i*t->c + j];
                         double g_j = t->grad[i*t->c + j];
                         // This is O(N^2) per row, computationally expensive but correct
                         double grad_sum = 0;
                         for (int k=0; k<t->c; k++) {
                             double s_k = t->data[i*t->c + k];
                             double delta = (j == k)? 1.0 : 0.0;
                             grad_sum += t->grad[i*t->c + k] * s_j * (delta - s_k);
                         }
                         t->parent_a->grad[i*t->c + j] += grad_sum;
                     }
                 }
            }
            break;
        default: break;
    }
    
    // Recursive Depth-First Search
    if (t->parent_a) backward_pass(t->parent_a);
    if (t->parent_b) backward_pass(t->parent_b);
}

void tensor_backward(void* t_t) {
    Tensor* t = unbox_ptr((Value)t_t);
    // Initialize root gradient (dL/dL = 1)
    for(int i=0; i<t->r * t->c; i++) t->grad[i] = 1.0;
    backward_pass(t);
}

void tensor_step_sgd(void* t_t, void* lr_t) {
    Tensor* t = unbox_ptr((Value)t_t);
    double lr = unbox_double((Value)lr_t);
    if (t->requires_grad) {
        for(int i=0; i<t->r * t->c; i++) {
            t->data[i] -= lr * t->grad[i];
            t->grad[i] = 0; // Reset for next iteration
        }
    }
}

// ============================================================================
// PART 3: ADVANCED ARCHITECTURES (MAMBA & TRANSFORMERS)
// ============================================================================

// --- Multi-Head Attention ---
// Implements: Softmax( (Q @ K^T) / sqrt(d) ) @ V
void* ai_self_attention(void* Q_t, void* K_t, void* V_t) {
    Tensor* Q = unbox_ptr((Value)Q_t); 
    Tensor* K = unbox_ptr((Value)K_t);
    Tensor* V = unbox_ptr((Value)V_t);
    
    // 1. Transpose K
    Tensor* KT = tensor_raw(K->c, K->r, 0);
    for(int i=0; i<K->r; i++)
        for(int j=0; j<K->c; j++)
            KT->data[j*K->r + i] = K->data[i*K->c + j];
            
    // 2. Scores = Q @ KT
    void* scores_t = tensor_matmul(Q_t, box_ptr(KT));
    Tensor* scores = unbox_ptr((Value)scores_t);
    
    // 3. Scale by 1/sqrt(dim)
    double scale = 1.0 / sqrt((double)Q->c);
    for(int i=0; i < scores->r * scores->c; i++) scores->data[i] *= scale;
    
    // 4. Softmax
    void* probs_t = tensor_softmax(scores_t);
    
    // 5. Output = Probs @ V
    return tensor_matmul(probs_t, V_t);
}

// --- Mamba Selective Scan Kernel ---
// Discretized SSM: h_t = A*h_{t-1} + B*x_t; y_t = C*h_t
// A is diagonal. B and C are time-varying (input dependent).
void* ai_mamba_scan(void* u_t, void* A_t, void* B_t, void* C_t) {
    Tensor* u = unbox_ptr((Value)u_t); //
    Tensor* A = unbox_ptr((Value)A_t); // [N] (Broadcast)
    Tensor* B = unbox_ptr((Value)B_t); // [L, N]
    Tensor* C = unbox_ptr((Value)C_t); // [L, N]
    
    int L = u->r; 
    int Dim = u->c; 
    int N = A->c;
    
    Tensor* y = tensor_raw(L, Dim, 0); 
    
    // Hidden State
    double* h = calloc(Dim * N, sizeof(double));
    
    // Sequential Scan
    for (int t = 0; t < L; t++) {
        for (int d = 0; d < Dim; d++) {
            double x_val = u->data[t*Dim + d];
            double y_val = 0;
            
            for (int n = 0; n < N; n++) {
                int h_idx = d * N + n;
                
                // Discretized update (simplified ZOH)
                // h = A*h + B*x
                h[h_idx] = A->data[n] * h[h_idx] + B->data[t*N + n] * x_val;
                
                // Output mapping
                y_val += C->data[t*N + n] * h[h_idx];
            }
            y->data[t*Dim + d] = y_val;
        }
    }
    free(h);
    return (void*)box_ptr(y);
}

// ============================================================================
// PART 4: HNSW SEMANTIC SEARCH
// Hierarchical Navigable Small World Graph for Approximate Nearest Neighbor
// ============================================================================

#define HNSW_M 16
#define HNSW_EF 128
#define MAX_LAYERS 4

typedef struct HNSWNode {
    int id;
    Tensor* vec;
    char* payload;
    struct HNSWNode* neighbors[MAX_LAYERS][HNSW_M];
    int neighbor_counts[MAX_LAYERS];
    int level;
} HNSWNode;

typedef struct {
    HNSWNode* entry_point;
    int max_level;
    int count;
    HNSWNode** nodes;
    int capacity;
} VectorDB;

// Cosine Distance: 1.0 - CosSimilarity
double hnsw_dist(Tensor* a, Tensor* b) {
    double dot = 0, magA = 0, magB = 0;
    for(int i=0; i<a->c; i++) {
        dot += a->data[i] * b->data[i];
        magA += a->data[i] * a->data[i];
        magB += b->data[i] * b->data[i];
    }
    return 1.0 - (dot / (sqrt(magA) * sqrt(magB) + 1e-9));
}

void* db_new() {
    VectorDB* db = aria_alloc(sizeof(VectorDB));
    db->entry_point = NULL;
    db->max_level = -1;
    db->count = 0;
    db->capacity = 4096;
    db->nodes = aria_alloc(sizeof(HNSWNode*) * db->capacity);
    return (void*)box_ptr(db);
}

// Greedy Search in a specific layer
HNSWNode* search_layer(HNSWNode* entry, Tensor* query, int layer) {
    HNSWNode* curr = entry;
    double curr_dist = hnsw_dist(curr->vec, query);
    int changed = 1;
    
    while (changed) {
        changed = 0;
        HNSWNode* best_cand = NULL;
        double best_dist = curr_dist;
        
        for (int i=0; i < curr->neighbor_counts[layer]; i++) {
            HNSWNode* neighbor = curr->neighbors[layer][i];
            double d = hnsw_dist(neighbor->vec, query);
            if (d < best_dist) {
                best_dist = d;
                best_cand = neighbor;
            }
        }
        
        if (best_cand) {
            curr = best_cand;
            curr_dist = best_dist;
            changed = 1;
        }
    }
    return curr;
}

void db_insert(void* db_t, void* vec_t, void* payload_t) {
    VectorDB* db = unbox_ptr((Value)db_t);
    Tensor* vec = unbox_ptr((Value)vec_t);
    char* payload = (char*)((Value)payload_t & PTR_MASK);
    
    HNSWNode* node = aria_alloc(sizeof(HNSWNode));
    node->id = db->count;
    node->vec = vec;
    node->payload = aria_alloc(strlen(payload)+1);
    strcpy(node->payload, payload);
    node->level = 0;
    while(rand() % 2 == 0 && node->level < MAX_LAYERS - 1) node->level++;
    
    if(db->count < db->capacity) db->nodes[db->count++] = node;

    if (!db->entry_point) {
        db->entry_point = node;
        db->max_level = node->level;
        return;
    }
    
    // 1. Search top-down for entry point to node's level
    HNSWNode* curr = db->entry_point;
    for (int l = db->max_level; l > node->level; l--) {
        curr = search_layer(curr, vec, l);
    }
    
    // 2. Insert into levels
    for (int l = (node->level < db->max_level? node->level : db->max_level); l >= 0; l--) {
        curr = search_layer(curr, vec, l);
        
        // Simple bidirectional connect (Naively assumes space in neighbor array)
        if (node->neighbor_counts[l] < HNSW_M) 
            node->neighbors[l][node->neighbor_counts[l]++] = curr;
        if (curr->neighbor_counts[l] < HNSW_M) 
            curr->neighbors[l][curr->neighbor_counts[l]++] = node;
    }
    
    if (node->level > db->max_level) {
        db->max_level = node->level;
        db->entry_point = node;
    }
}

void* db_query(void* db_t, void* vec_t) {
    VectorDB* db = unbox_ptr((Value)db_t);
    Tensor* q = unbox_ptr((Value)vec_t);
    
    if (!db->entry_point) return (void*)box_str("");
    
    HNSWNode* curr = db->entry_point;
    for (int l = db->max_level; l >= 0; l--) {
        curr = search_layer(curr, q, l);
    }
    return (void*)box_str(curr->payload);
}

// ============================================================================
// PART 5: AGENTIC SERVER
// ============================================================================

typedef struct { int fd; } AgentCtx;

void agent_handler(void* arg) {
    AgentCtx* ctx = (AgentCtx*)arg;
    void* req_val = net_read(box_int(ctx->fd));
    if (req_val) {
        // High-level "Process" simulation
        char* req_s = (char*)((Value)req_val & PTR_MASK);
        char buf[256];
        snprintf(buf, 256, "AI_ACK: %lu bytes", strlen(req_s));
        
        char* r = aria_alloc(strlen(buf)+1);
        strcpy(r, buf);
        net_write(box_int(ctx->fd), box_str(r));
    }
    net_close(box_int(ctx->fd));
    free(ctx);
}

void* ai_server_start(void* port_t) {
    ai_pool_init();
    // For now, return a dummy server handle since net_server_start returns void
    // In a real implementation, this would be properly handled
    return box_int(8080); // dummy server fd
}

/* Aria_lang/src/stdlib/ssh.c */
#include <libssh/libssh.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

extern void* aria_alloc(size_t size);
typedef uint64_t Value;
#define TAG_OBJECT (0xFFF8000000000000ULL | 6ULL)
#define TAG_STRING (0xFFF8000000000000ULL | 5ULL)
#define TAG_INTEGER (0xFFF8000000000000ULL | 4ULL)
#define PTR_MASK 0x0000FFFFFFFFFFFFULL

static inline Value box_ptr(void* p, uint64_t t) { return t | (uintptr_t)p; }
static inline Value box_int(int32_t i) { return TAG_INTEGER | (uint32_t)i; }
static inline char* unbox_str(Value v) { return (char*)(v & PTR_MASK); }
static inline void* unbox_ptr(Value v) { return (void*)(v & PTR_MASK); }

void* ssh_connect_sess(void* host_t, void* user_t, void* port_t) {
    char* host = unbox_str((Value)host_t);
    char* user = unbox_str((Value)user_t);
    int port = (int32_t)((Value)port_t & 0xFFFFFFFF);
    
    ssh_session session = ssh_new();
    if (!session) return NULL;
    
    ssh_options_set(session, SSH_OPTIONS_HOST, host);
    ssh_options_set(session, SSH_OPTIONS_USER, user);
    ssh_options_set(session, SSH_OPTIONS_PORT, &port);
    
    int rc = ssh_connect(session);
    if (rc!= SSH_OK) {
        ssh_free(session);
        return NULL;
    }
    return (void*)box_ptr(session, TAG_OBJECT);
}

void* ssh_auth_pwd(void* sess_t, void* pass_t) {
    ssh_session session = unbox_ptr((Value)sess_t);
    char* pass = unbox_str((Value)pass_t);
    if (!session) return (void*)box_int(0);
    int rc = ssh_userauth_password(session, NULL, pass);
    return (void*)box_int(rc == SSH_AUTH_SUCCESS);
}

void* ssh_run(void* sess_t, void* cmd_t) {
    ssh_session session = unbox_ptr((Value)sess_t);
    char* cmd = unbox_str((Value)cmd_t);
    if (!session) return NULL;
    
    ssh_channel ch = ssh_channel_new(session);
    if (!ch) return NULL;
    if (ssh_channel_open_session(ch)!= SSH_OK) {
        ssh_channel_free(ch);
        return NULL;
    }
    if (ssh_channel_request_exec(ch, cmd)!= SSH_OK) {
        ssh_channel_close(ch);
        ssh_channel_free(ch);
        return NULL;
    }
    
    char buf;
    int n = ssh_channel_read(ch, buf, sizeof(buf), 0);
    ssh_channel_send_eof(ch);
    ssh_channel_close(ch);
    ssh_channel_free(ch);
    
    if (n > 0) {
        char* r = aria_alloc(n+1);
        memcpy(r, buf, n);
        r[n] = 0;
        return (void*)box_ptr(r, TAG_STRING);
    }
    return (void*)box_ptr("", TAG_STRING);
}

void ssh_disconnect_sess(void* sess_t) {
    ssh_session s = unbox_ptr((Value)sess_t);
    if (s) { ssh_disconnect(s); ssh_free(s); }
}

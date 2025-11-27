/* Aria_lang/src/stdlib/sound.c */
#include <alsa/asoundlib.h>
#include <pthread.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

// --- Runtime Imports ---
typedef uint64_t Value;
#define TAG_INTEGER (0xFFF8000000000000ULL | 4ULL)
static inline int32_t unbox_int(Value v) { return (int32_t)(v & 0xFFFFFFFF); }

// --- Audio Constants ---
#define SAMPLE_RATE 44100
#define BUFFER_SIZE 2048 // Smaller buffer = lower latency
#define MAX_VOICES 16

// Voice Structure: Represents a single active note
typedef struct {
    float freq;
    float amp;
    int duration_samples;
    int current_sample;
    int active;
} Oscillator;

// Global State
static Oscillator voices;
static pthread_mutex_t mix_lock;
static pthread_t audio_thread;
static int running = 0;
static snd_pcm_t* pcm_handle;

// --- Mixer Thread ---
// This runs parallel to the Aria VM.
void* audio_loop(void* arg) {
    (void)arg;
    short buffer;
    
    while (running) {
        // 1. Zero out buffer (Silence)
        for(int i=0; i<BUFFER_SIZE; i++) buffer[i] = 0;
        
        // 2. Mix active voices
        pthread_mutex_lock(&mix_lock);
        
        // For every sample in the buffer...
        for(int i=0; i<BUFFER_SIZE; i++) {
            float sample_accum = 0.0f;
            
            // For every voice...
            for(int v=0; v<MAX_VOICES; v++) {
                if (voices[v].active) {
                    float t = (float)(voices[v].current_sample) / SAMPLE_RATE;
                    
                    // Generate Sine Wave: sin(2 * pi * f * t)
                    sample_accum += sinf(2.0f * M_PI * voices[v].freq * t) * voices[v].amp;
                    
                    // Advance voice time
                    voices[v].current_sample++;
                    if (voices[v].current_sample >= voices[v].duration_samples) {
                        voices[v].active = 0; // Note finished
                    }
                }
            }
            
            // 3. Hard Limiter / Clipping
            // Prevent wrapping artifacts by clamping to [-1.0, 1.0]
            if (sample_accum > 1.0f) sample_accum = 1.0f;
            if (sample_accum < -1.0f) sample_accum = -1.0f;
            
            // Convert to 16-bit PCM
            buffer[i] = (short)(sample_accum * 32000.0f);
        }
        pthread_mutex_unlock(&mix_lock);
        
        // 4. Write to ALSA Hardware
        // This call blocks until the hardware consumes the buffer.
        snd_pcm_sframes_t frames = snd_pcm_writei(pcm_handle, buffer, BUFFER_SIZE);
        
        // Handle Underruns (Buffer empty too fast)
        if (frames < 0) {
            snd_pcm_recover(pcm_handle, frames, 0);
        }
    }
    return NULL;
}

// --- Public API ---

// sound_init() -> Success(1) | Failure(0)
void* sound_init() {
    // Open PCM device for playback
    if (snd_pcm_open(&pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0) {
        fprintf(stderr, "Audio Error: ALSA open failed.\n");
        return (void*)0;
    }
    
    // Set parameters: S16_LE, Interleaved, 44100Hz
    if (snd_pcm_set_params(pcm_handle, 
                           SND_PCM_FORMAT_S16_LE, 
                           SND_PCM_ACCESS_RW_INTERLEAVED, 
                           1, // Channels (Mono for simplicity)
                           SAMPLE_RATE, 
                           1, 
                           500000) < 0) { // Latency
        fprintf(stderr, "Audio Error: ALSA params failed.\n");
        return (void*)0;
    }
    
    pthread_mutex_init(&mix_lock, NULL);
    
    // Initialize voices
    for(int i=0; i<MAX_VOICES; i++) voices[i].active = 0;
    
    // Start Thread
    running = 1;
    if (pthread_create(&audio_thread, NULL, audio_loop, NULL)!= 0) {
        fprintf(stderr, "Audio Error: Thread creation failed.\n");
        return (void*)0;
    }
    
    return (void*)1;
}

// sound_play_tone(freq, ms)
// Finds a free voice slot and schedules the note.
void sound_play_tone(void* freq_t, void* ms_t) {
    int freq = unbox_int((Value)freq_t);
    int ms = unbox_int((Value)ms_t);
    
    pthread_mutex_lock(&mix_lock);
    // Find first inactive voice
    for(int i=0; i<MAX_VOICES; i++) {
        if (!voices[i].active) {
            voices[i].freq = (float)freq;
            voices[i].amp = 0.5f; // 50% Volume
            voices[i].duration_samples = (SAMPLE_RATE * ms) / 1000;
            voices[i].current_sample = 0;
            voices[i].active = 1;
            break;
        }
    }
    pthread_mutex_unlock(&mix_lock);
}

// sound_close()
// Stop thread and release device.
void sound_close() {
    running = 0;
    pthread_join(audio_thread, NULL); // Wait for thread to finish
    snd_pcm_close(pcm_handle);
    pthread_mutex_destroy(&mix_lock);
}

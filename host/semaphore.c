#include "semaphore.h"

#include <pthread.h>
#include "constants.h"
#include "screen.h"
#include "serial.h"

pthread_mutex_t semaphore_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t semaphore_cond     = PTHREAD_COND_INITIALIZER;

void *semaphore_update(void *arg) {
    int *keep_running = (int *) arg;

    char semaphore_color;
    while(*keep_running) {
        // DEBUG_PRINT("f update_semaphore block\n");

        // Block until other thread wake me up
        pthread_cond_wait(&semaphore_cond, &semaphore_mutex);

        // Other thread may want to terminate this thread
        if(!*keep_running) break;

        // Copy to prevent other thread modify serial_rx_buffer
        semaphore_color = serial_rx_buffer;

        if(semaphore_color != RED && semaphore_color != YELLOW && semaphore_color != GREEN) {
            continue;
        }
        
        screen_set_semaphore(semaphore_color);
    }
    return 0;
}

int semaphore_char_is_allowed(char character) {
    return character == RED || character == YELLOW || character == GREEN;
}

void semaphore_signal(void) {
    pthread_cond_signal(&semaphore_cond);
}

void semaphore_destroy(void) {
    pthread_mutex_destroy(&semaphore_mutex);
    pthread_cond_destroy(&semaphore_cond);
}
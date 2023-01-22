#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>

#include "constants.h"
#include "semaphore.h"
#include "emergency.h"
#include "serial.h"
#include "screen.h"

static volatile int keep_running = TRUE;

int main() {
    pthread_t thread[4];
    int thread_ret_code;

    int keep_running = TRUE;

    thread_ret_code = pthread_create(&thread[0], NULL, semaphore_update, (void *) &keep_running);
    if(thread_ret_code != 0) {
        printf("Error creating semaphore_update thread. Code: %i", thread_ret_code);
        return -1;
    }

    thread_ret_code = pthread_create(&thread[2], NULL, emergency_manage, (void *) &keep_running);
    if(thread_ret_code != 0) {
        printf("Error creating emergency_manage thread. Code: %i", thread_ret_code);
        return -1;
    }

    thread_ret_code = pthread_create(&thread[3], NULL, screen_user_input, (void *) &keep_running);
    if(thread_ret_code != 0) {
        printf("Error creating screen_user_input thread. Code: %i", thread_ret_code);
        return -1;
    }

    thread_ret_code = pthread_create(&thread[1], NULL, serial_read, (void *) &keep_running);
    if(thread_ret_code != 0) {
        printf("Error creating serial_read thread. Code: %i", thread_ret_code);
        return -1;
    }

    pthread_join(thread[0], NULL);
    pthread_join(thread[2], NULL);
    pthread_join(thread[3], NULL);

    pthread_kill(thread[1], SIGINT);
    pthread_join(thread[1], NULL);

    serial_destroy();

    return 0;
}
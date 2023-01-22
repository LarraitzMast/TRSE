#include "emergency.h"

#include <pthread.h>
#include <string.h>
#include "constants.h"
#include "serial.h"
#include "screen.h"

#define AMBULANCE_BLOCK         'E'
#define TOTAL_BLOCK             'F'
#define BLOCK_ACK               'K'

#define ASK_TO_EMERGENCY_TYPE   '?'
#define UNBLOCK                 'H'

#define EMERGENCY_NUMBER        "112"

#define SIRENA_ON               'I'
#define SIRENA_OFF              'J'

#define LISTEN_TO_TLF_NUMBER        0
#define LISTEN_TO_EMERGENCY_TYPE    1
#define LISTEN_FOR_BLOCK_ACK        2
#define BLOCKED                     3


pthread_mutex_t emergency_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t emergency_cond   = PTHREAD_COND_INITIALIZER;

static int emergency_state = LISTEN_TO_TLF_NUMBER;


static int is_tlf_digit(char character) {
    return character >= '0' && character <= '9';
}

static int is_emergency_type(char character) {
    return character == EMERGENCY_TYPE_A || character == EMERGENCY_TYPE_B;
}

static int is_block_ack(char character) {
    return character == BLOCK_ACK;
}


static int manage_tlf(char tlf_number[], char new_digit) {
    if(!is_tlf_digit(new_digit)) {
        return LISTEN_TO_TLF_NUMBER;
    }

    if(strlen(tlf_number) > 9) {
        strcpy(tlf_number, &new_digit); // Overwrite the previous number
        return LISTEN_TO_TLF_NUMBER;
    }

    // Concat the digit to the number
    char digitString[2] = { new_digit, 0 };
    strcat(tlf_number, digitString);

    screen_tlf(tlf_number);

    if(strcmp(tlf_number, EMERGENCY_NUMBER) != 0) {
        // The number is not complete or its not known
        return LISTEN_TO_TLF_NUMBER;        
    }

    serial_write(ASK_TO_EMERGENCY_TYPE);

    screen_ask_emergency();
    screen_debug("[EMERGENCY] LISTEN_TO_EMERGENCY_TYPE", 2);

    strcpy(tlf_number, ""); // Clean the number
    return LISTEN_TO_EMERGENCY_TYPE;
}

static int manage_emergency_type(char emergency_type) {
    if(!is_emergency_type(emergency_type)) {
        return LISTEN_TO_EMERGENCY_TYPE;
    }

    if(emergency_type == 'A')
        serial_write(AMBULANCE_BLOCK);
    else if(emergency_type == 'B')
        serial_write(TOTAL_BLOCK);

    // serial_write(emergency_type);
    screen_set_emergency(emergency_type);
    screen_debug("[EMERGENCY] LISTEN_FOR_BLOCK_ACK", 2);

    return LISTEN_FOR_BLOCK_ACK;
}

static int manage_block_ack(char block_ack) {
    if(!is_block_ack(block_ack)) {
        return LISTEN_FOR_BLOCK_ACK;
    }

    screen_blocked();
    screen_debug("[EMERGENCY] BLOCKED                   ", 2);
    screen_enable_unblocking();

    return BLOCKED;
}

void emergency_unblock(void) {
    if(BLOCKED) {
        serial_write(UNBLOCK);
        screen_disable_unblocking();
        screen_reset_emergency();

        emergency_state = LISTEN_TO_TLF_NUMBER;
        screen_msg_clean();
        screen_debug("[EMERGENCY] LISTEN_TO_TLF_NUMBER", 2);
    }
}


void *emergency_manage(void *arg) {
    int *keep_running = (int *) arg;

    char current_tlf_number[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};    // 9 digits and '\0'
    char current_rx_buffer, new_digit, emergency_type, block_ack;

    // screen_debug("[EMERGENCY] LISTEN_TO_TLF_NUMBER", 2);

    //int n_iterations = 0;
    while(*keep_running) {
        // DEBUG_PRINT("f manage_emergency block\n");

        // Block until other thread wake me up
        pthread_cond_wait(&emergency_cond, &emergency_mutex);

        // Other thread may want to terminate this thread
        if(!*keep_running) break;

        current_rx_buffer = serial_rx_buffer;

        switch(emergency_state) {
            case LISTEN_TO_TLF_NUMBER:
                new_digit = current_rx_buffer;
                emergency_state = manage_tlf(current_tlf_number, new_digit);
                break;

            case LISTEN_TO_EMERGENCY_TYPE:
                emergency_type = current_rx_buffer;
                emergency_state = manage_emergency_type(emergency_type);
                break;

            case LISTEN_FOR_BLOCK_ACK:
                block_ack = current_rx_buffer;
                emergency_state = manage_block_ack(block_ack);
                break;

            case BLOCKED:
                break;
        }

        //n_iterations++;
    }
    return 0;
}

int emergency_char_is_allowed(char character) {
    return is_tlf_digit(character) ||
            is_emergency_type(character) ||
            is_block_ack(character);
}

// void emergency_unlock(void) {
//     if(BLOCKED) {
//         emergency_state = LISTEN_TO_TLF_NUMBER;
//         emergency_signal();
//         serial_write(UNLOCK);
//     }
// }

int ambulance_char_is_allowed(char character) {
    return character == SIRENA_ON || character == SIRENA_OFF;
}

void handleAmbulanceChar(char character) {
    if(character == SIRENA_ON)
        screen_turn_on_ambulance();
    else if(character == SIRENA_OFF)
        screen_turn_off_ambulance();
}

void emergency_signal(void) {
    pthread_cond_signal(&emergency_cond);
}

void emergency_destroy(void) {
    pthread_mutex_destroy(&emergency_mutex);
    pthread_cond_destroy(&emergency_cond);
}
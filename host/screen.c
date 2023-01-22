#include "screen.h"

#include <stdio.h>
#include <unistd.h>
#include <locale.h>
#include <ncurses.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include "constants.h"
#include "emergency.h"
#include "semaphore.h"
#include "serial.h"

#define NORMAL_COLOR    1
#define EMERGENCY_COLOR 2

#define EM_RED      "🟥"
#define EM_YELLOW   "🟧"
#define EM_GREEN    "🟩"
#define EM_BLUE     "🟦"
#define EM_CLEAN    "  "

#define EM_SIREN    "🚨🚨🚨"
#define EM_CLEAN3   "       "

#define SEM_X           39
#define SEM_RED_Y       5
#define SEM_YELLOW_Y    7
#define SEM_GREEN_Y     9

#define AMBULANCE_X     12
#define AMBULANCE_Y     6

#define EMERGENCY_X     63
#define EMERGENCY_A_Y   6
#define EMERGENCY_B_Y   8

#define SCREEN_SIZE 22
static char *screen_template[SCREEN_SIZE] = {
    "╔═══════════════════════════╦══════════════════════╦═══════════════════════════╗",
    "║    []     {}     ()       ║   PANEL DE CONTROL   ║       ()     {}     []    ║",
    "║                          ┌╚══════════════════════╝┐                          ║",
    "║  ┌───────────────────────┤     Semáforo actual    ├───────────────────────┐  ║",
    "║──┤      Ambulancia       │         ┌────┐         │    Tipo emergencia    ├──║",
    "║──┤      ┌────────┐       │         │    │         │         ┌───┐         ├──║",
    "║──┤      │        │       │         ├────┤         │         │ A │         ├──║",
    "║──┤      └────────┘       │         │    │         │         ├───┤         ├──║",
    "║  └───────────────────────┤         ├────┤         │         │ B │         ├──║",
    "║                          │         │    │         │         └───┘         ├──║",
    "║                          │         └────┘         ├───────────────────────┘  ║",
    "║                          └────────────────────────┘                          ║",
    "║                                                                              ║",
    "║                                                                              ║",
    "║                                                                              ║",
    "║                                                                              ║",
    "║                                                                              ║",
    "║                                                                              ║",
    "║                                ╭───────────╮                                 ║",
    "║                                │ SALIR [S] │                                 ║",
    "║                                ╰───────────╯                                 ║",
    "╚══════════════════════════════════════════════════════════════════════════════╝"
};

#define UNBLOCKING_BUTTON_SIZE  3
#define UNBLOCKING_BUTTON_Y     18
#define UNBLOCKING_BUTTON_X     57
static int is_blocking_enable = FALSE;
static char *unblocking_button[UNBLOCKING_BUTTON_SIZE] = {
    "╭────────────────╮",
    "│ DESBLOQUEO [D] │",
    "╰────────────────╯"
};

static char *unblocking_clean[UNBLOCKING_BUTTON_SIZE] = {
    "                  ",
    "                  ",
    "                  "
};

int logFile;

static void test_all(void) {
    screen_set_semaphore(RED);
    screen_set_emergency(EMERGENCY_TYPE_A);
    sleep(2);

    screen_turn_on_ambulance();
    screen_set_semaphore(YELLOW);
    screen_set_emergency(EMERGENCY_TYPE_B);
    sleep(2);

    screen_turn_off_ambulance();
    screen_set_semaphore(GREEN);

    sleep(2);
}

static int handle_key(int user_key) {
    int keep_running = TRUE;
    switch(user_key) {
        case 'S':
        case 's':
            keep_running = FALSE;
            break;

        case 'D':
        case 'd':
            if(is_blocking_enable) {
                emergency_unblock();
            }
            break;

        // case 'b':
        //     if(is_blocking_enable)
        //         screen_disable_unblocking();
        //     else
        //         screen_enable_unblocking();
        //     break;
    }
    return keep_running;
}

void screen_init(void) {
    setlocale(LC_ALL, "");

    initscr();

    noecho();
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_BLACK, COLOR_YELLOW);

    curs_set(0);

    for(int i = 0; i < SCREEN_SIZE; i++) {
        printw("%s\n", screen_template[i]);
    }
    refresh();

    logFile = open("logfile.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
}

void screen_destroy(void) {
    endwin();

    close(logFile);

    semaphore_signal();
    emergency_signal();
}

void *screen_user_input(void *arg) {
    screen_init();

    serial_signal();

    int *keep_running = (int *) arg;
    int user_key;
    while(*keep_running) {
        user_key = getch();
        *keep_running = handle_key(user_key);
    }

    screen_destroy();
    return 0;
}

void screen_reset_semaphore(void) {
    move(SEM_RED_Y, SEM_X);
    printw(EM_CLEAN);

    move(SEM_YELLOW_Y, SEM_X);
    printw(EM_CLEAN);

    move(SEM_GREEN_Y, SEM_X);
    printw(EM_CLEAN);

    refresh();
}

void screen_set_semaphore(char semaphore_color) {
    screen_reset_semaphore();
    switch(semaphore_color) {
        case RED:
            move(SEM_RED_Y, SEM_X);
            printw(EM_RED);
            break;
        
        case YELLOW:
            move(SEM_YELLOW_Y, SEM_X);
            printw(EM_YELLOW);
            break;
        
        case GREEN:
            move(SEM_GREEN_Y, SEM_X);
            printw(EM_GREEN);
            break;
    }

    refresh();
}

void screen_turn_on_ambulance(void) {
    move(AMBULANCE_Y, AMBULANCE_X);
    printw(EM_SIREN);
    refresh();
}

void screen_turn_off_ambulance(void) {
    move(AMBULANCE_Y, AMBULANCE_X);
    printw(EM_CLEAN3);
    refresh();
}

void screen_reset_emergency(void) {
    move(EMERGENCY_A_Y, EMERGENCY_X);
    printw(" A ");

    move(EMERGENCY_B_Y, EMERGENCY_X);
    printw(" B ");
}

void screen_set_emergency(char emergency_type) {
    screen_reset_emergency();

    attron(COLOR_PAIR(EMERGENCY_COLOR));
    switch(emergency_type) {
        case EMERGENCY_TYPE_A:
            move(EMERGENCY_A_Y, EMERGENCY_X);
            printw(" A ");
            break;
        case EMERGENCY_TYPE_B:
            move(EMERGENCY_B_Y, EMERGENCY_X);
            printw(" B ");
            break;
    }
    attron(COLOR_PAIR(NORMAL_COLOR));
}

void screen_enable_unblocking(void) {
    if(is_blocking_enable) return;

    for(int i = 0; i < UNBLOCKING_BUTTON_SIZE; i++) {
        move(UNBLOCKING_BUTTON_Y + i, UNBLOCKING_BUTTON_X);
        printw(unblocking_button[i]);
    }
    refresh();

    is_blocking_enable = TRUE;
}

void screen_disable_unblocking(void) {
    if(!is_blocking_enable) return;

    for(int i = 0; i < UNBLOCKING_BUTTON_SIZE; i++) {
        move(UNBLOCKING_BUTTON_Y + i, UNBLOCKING_BUTTON_X);
        printw(unblocking_clean[i]);
    }
    refresh();

    is_blocking_enable = FALSE;
}

void screen_tlf(char *msg) {
    char msgToPrint[50] = "Llamada entrante: ";
    strcat(msgToPrint, msg);
    move(13, 2);
    printw(msgToPrint);
    refresh();
}

void screen_ask_emergency() {
    char msgToPrint[50] = "¿Que tipo de emergencia es?";
    move(13, 2);
    printw(msgToPrint);
    refresh();
}

void screen_blocked() {
    char msgToPrint[50] = "Sistemas bloqueados         ";
    move(13, 2);
    printw(msgToPrint);
    refresh();
}

void screen_msg_clean() {
    char msgToPrint[50] = "                                ";
    move(13, 2);
    printw(msgToPrint);
    refresh();
}

void screen_debug(char *msg, int position) {
    move(14 + position, 2);
    printw(msg);
    refresh();

    // char msgToLog[50];
    // strcpy(msgToLog, msg);
    // strcat(msgToLog, "\n");
    // write(logFile, msgToLog, strlen(msgToLog));
}
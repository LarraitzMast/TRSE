#include "serial.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include "constants.h"
#include "semaphore.h"
#include "emergency.h"
#include "screen.h"

char serial_rx_buffer = 0;
char serial_tx_buffer = 0;

static int serial_port_fd;


pthread_mutex_t serial_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t serial_cond   = PTHREAD_COND_INITIALIZER;

void serial_init(void) {
    serial_port_fd = open("/dev/ttyACM0", O_RDWR);
    if(serial_port_fd < 0) {
        printf("Error %i from open: %s\n", errno, strerror(errno));
        exit(errno);
    }

    struct termios tty;

    // Read in existing settings and handle any error
    if(tcgetattr(serial_port_fd, &tty) != 0) {
        printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
        exit(errno);
    }

    // Control modes
    tty.c_cflag &= ~PARENB;         // Disable parity
    tty.c_cflag &= ~CSTOPB;         // 1 stop bit
    tty.c_cflag &= ~CSIZE;          // Clear all the size bits
    tty.c_cflag |= CS8;             // Use 8 bits
    tty.c_cflag &= ~CRTSCTS;        // Disable RTS/CTS hardware flow control
    tty.c_cflag |= CREAD | CLOCAL;  // Turn on READ & ignore ctrl lines

    // Local modes
    tty.c_lflag &= ~ICANON;         // Disable canonical mode
    tty.c_lflag &= ~ECHO;           // Disable echo
    tty.c_lflag &= ~ECHOE;          // Disable erasure
    tty.c_lflag &= ~ECHONL;         // Disable new-line echo
    tty.c_lflag &= ~ISIG;           // Disable interpretation of INTR, QUIT and SUSP

    // Input modes
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes

    // Output modes
    tty.c_oflag &= ~OPOST;          // Prevent special interpretatil of output bytes
    tty.c_oflag &= ~ONLCR;          // Prevent conversion of newline to carriage return/line feed

    // read() block until VMIN bytes get from serial
    // With 0 timeout (VITME), this could block indefinitely
    tty.c_cc[VTIME] = 0;
    tty.c_cc[VMIN] = 1;

    // Baud rate
    cfsetispeed(&tty, B115200);
    cfsetospeed(&tty, B115200);

    // Save tty setting
    if(tcsetattr(serial_port_fd, TCSANOW, &tty) != 0) {
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
        exit(errno);
    }
}

void serial_destroy(void) {
    close(serial_port_fd);
}

void serial_signal() {
    pthread_cond_signal(&serial_cond);
}

void *serial_read(void *arg) {
    // pthread_cond_wait(&serial_cond, &serial_mutex);
    serial_init();

    sleep(2); //required to make flush work, for some reason
    tcflush(serial_port_fd, TCIOFLUSH);

    int *keep_running = (int *) arg;

    char read_buffer[2] = {'\0', '\0'}; // Size 2 to avoid \n
    int n_read;

    char msg[80];

    while(*keep_running) {

        n_read = read(serial_port_fd, &read_buffer, sizeof(uint8_t));

        if(n_read == 0) continue;

        serial_rx_buffer = read_buffer[0];

        sprintf(msg, "[RX] %c", serial_rx_buffer);
        screen_debug(msg, 0);

        if(semaphore_char_is_allowed(serial_rx_buffer)) {
            semaphore_signal();
        } else if(emergency_char_is_allowed(serial_rx_buffer)) {
            emergency_signal();
        } else if(ambulance_char_is_allowed(serial_rx_buffer)) {
            handleAmbulanceChar(serial_rx_buffer);
        }
    }

    serial_destroy();

    return 0;
}

void serial_write(char character) {
    write(serial_port_fd, &character, sizeof(uint8_t));

    char msg[80];
    sprintf(msg, "[TX] %c", character);
    screen_debug(msg, 1);
}
#ifndef SERIAL_H
#define SERIAL_H

extern char serial_rx_buffer;
extern char serial_tx_buffer;

/*
 *  Configure the serial port
 *  - 115200 bauds
 *  - 8 bits
 *  - No parity
 *  - 1 stop bit
 *
 * Return the file descriptor of the serial
 * or a negative number if an error happend
 */
void serial_init(void);

/*
 *  Destroy and close all objects 
 *  needed to the serial
 */
void serial_destroy(void);

void serial_signal();

void *serial_read(void *arg);

void serial_write(char character);

#endif
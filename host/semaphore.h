#ifndef SEMAPHORE_H
#define SEMAPHORE_H

/*
 *  Thread function that update the semaphore
 *  with the hardware data (from rx serial)
 */
void *semaphore_update(void *arg);

void semaphore_signal(void);

int semaphore_char_is_allowed(char character);

void semaphore_destroy(void);

#endif
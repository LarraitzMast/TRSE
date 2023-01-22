#ifndef EMERGENCY_H
#define EMERGENCY_H

static int is_tlf_digit(char character);
static int is_emergency_type(char character);
static int is_block_ack(char character);

static int manage_tlf(char tlf_number[], char new_digit); 
static int manage_emergency_type(char emergency_type);
static int manage_block_ack(char block_ack);

void emergency_unblock(void);
void *emergency_manage(void *arg);
int emergency_char_is_allowed(char character);
// void emergency_unlock(void);
void emergency_signal(void);
void emergency_destroy(void);

int ambulance_char_is_allowed(char character);
void handleAmbulanceChar(char character);

#endif
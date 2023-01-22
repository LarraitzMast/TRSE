#ifndef SCREEN_H
#define SCREEN_H


static void test_all(void);
static int handle_key(int user_key);

void screen_init(void);
void screen_destroy(void);

void *screen_user_input(void *arg);

void screen_reset_semaphore(void);
void screen_set_semaphore(char semaphore_color);

void screen_turn_on_ambulance(void);
void screen_turn_off_ambulance(void);

void screen_reset_emergency(void);
void screen_set_emergency(char blocking_type);

void screen_enable_unblocking(void);
void screen_disable_unblocking(void);

void screen_tlf(char *msg);
void screen_ask_emergency();
void screen_blocked();
void screen_msg_clean();
void screen_debug(char *msg, int position);

#endif
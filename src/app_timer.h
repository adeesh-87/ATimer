#include <stdint.h>

typedef enum {
	TIMER_REPEATING,
	TIMER_ONESHOT
} app_timer_behaviour;

#define MAX_SIMULTANEOUS_TIMERS 8

void app_timer_init(void);
void app_timer_deinit(void);
int app_timer_create(int tdiff, app_timer_behaviour behaviour);
void app_timer_stop(int handle);

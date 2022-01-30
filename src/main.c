#include <stdio.h>
#include <unistd.h>
#include <ATimer.h>

volatile int a = 0;

void callback1(ATimer_t *ptimer, void *x)
{
	a++;
	printf("callback for timer %s: fired\n", ATimer_getname(ptimer));
}

void callback2(ATimer_t *ptimer, void *x)
{
	a++;
	printf("callback for timer %s: fired\n", ATimer_getname(ptimer));
}

int main()
{
	ATimer_init();
	ATimer_start("timer1", 1, AT_REPEATING, callback1);
	ATimer_start("timer2", 2, AT_REPEATING, callback2);
	while (a < 5) {
		usleep(1000);
	}
	ATimer_deinit();
	printf("Tested\n");
}

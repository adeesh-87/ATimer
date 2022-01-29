#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

void printQ(void);

volatile int a = 0;

typedef struct ATimer_t {
	char name[21];
	void (*cb)(struct ATimer_t *, void *);
	struct timespec timeout;
	struct ATimer_t *next;
} ATimer_t;

ATimer_t *ATQ; // Pointer to the head of the list
ATimer_t *done; // Pointer to head of timers that need to be processed
struct timespec tnowTick;

pthread_cond_t wCond;
pthread_mutex_t wCondMutex = PTHREAD_MUTEX_INITIALIZER;

pthread_t t1, t2;
volatile int bye = 0;

void *ATimer_tick_thread(void *arg)
{
	printf("ATimer_tick_thread: start\n");
	// struct timespec tnow;
	while (bye < 1) {
		if (ATQ) {
			clock_gettime(CLOCK_REALTIME, &tnowTick);
			if (tnowTick.tv_sec >= ATQ->timeout.tv_sec 
				&& tnowTick.tv_nsec >= ATQ->timeout.tv_nsec) 
			{
				pthread_mutex_lock(&wCondMutex);
				pthread_cond_signal(&wCond);
				pthread_mutex_unlock(&wCondMutex);
			}
		}
		usleep(1000);
	}
	bye = 2;
	pthread_mutex_lock(&wCondMutex);
	pthread_cond_signal(&wCond);
	pthread_mutex_unlock(&wCondMutex);
	printf("ATimer_tick_thread: end\n");
}

void *ATimer_worker_thread(void *arg)
{
	printf("ATimer_worker_thread: start\n");
	while (bye < 2) {
		pthread_mutex_lock(&wCondMutex);
		pthread_cond_wait(&wCond, &wCondMutex);
		pthread_mutex_unlock(&wCondMutex);
		while (ATQ && tnowTick.tv_sec >= ATQ->timeout.tv_sec 
				&& tnowTick.tv_nsec >= ATQ->timeout.tv_nsec) {
			printf("Processing %s\n", ATQ->name);
			ATimer_t *d = ATQ;
			// Order of next 2 lines is critical, if we mix them up, then error
			// because done will remain incorrect in case queue contains just
			// 1 timer
			ATQ = ATQ->next;
			d->cb(d, NULL);
			free(d);
		}
		usleep(1000);
	}
	bye = 3;
	printf("ATimer_worker_thread: end\n");
}

void ATimer_init()
{
	ATQ = done = NULL;
	pthread_create(&t1, NULL, ATimer_tick_thread, NULL);
	pthread_create(&t2, NULL, ATimer_worker_thread, NULL);
	printf("ATimer_init: ATimer initialized\n");
}

void ATimer_deinit()
{
	printf("ATimer_deinit: Shutting down ATimer threads\n");
	bye = 1;
	while (bye < 3) {
		usleep(1000);
	}
	printf("ATimer_deinit: Cleaning up ATimer Queue\n");
	while (ATQ) {
		ATimer_t *h = ATQ;
		ATQ = ATQ->next;
		free(h);
	}
	printf("ATimer_deinit: ATimer Deinitialized\n");
}

void ATimer_start(char *name, int period, void (*cb)(ATimer_t *, void *))
{
	ATimer_t *new = malloc(sizeof(ATimer_t));
	new->next = ATQ;
	new->cb = cb;
	bzero(new->name, 21);
	strcpy(new->name, name);

	struct timespec tnow;
	clock_gettime(CLOCK_REALTIME, &tnow);
	new->timeout = tnow;
	new->timeout.tv_sec += period;
	printf("ATimer_start: %s\n", new->name);

	// Insert at the right place
	ATimer_t *p;
	while (new->next && new->timeout.tv_sec >= new->next->timeout.tv_sec) {
		p = new->next;
		new->next = new->next->next;
	}
	if (new->next == ATQ) {
		ATQ = new;
		printf("\t > Inserted @ head\n");
		return;
	}
	p->next = new;
	printf("\t > Inserted after %s\n", p->name);
	// printQ();
}

void printQ(void)
{
	ATimer_t *h = ATQ;
	while (h) {
		printf("%s->", h->name);
		h = h->next;
	}
	printf("NULL\n");
}

void callback1(ATimer_t *ptimer, void *x)
{
	a++;
	printf("callback for timer %s: fired\n", ptimer->name);
	ATimer_start("timer1", 1, callback1);
}

void callback2(ATimer_t *ptimer, void *x)
{
	a++;
	printf("callback for timer %s: fired\n", ptimer->name);
	ATimer_start("timer2", 2, callback2);
}

int main()
{
	ATimer_init();
	ATimer_start("timer1", 1, callback1);
	ATimer_start("timer2", 2, callback2);
	while (a < 5) {
		usleep(1000);
	}
	ATimer_deinit();
	printf("Tested\n");
}

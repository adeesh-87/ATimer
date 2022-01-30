#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <ATimer.h>

#define DBG_LEVEL	0

#if DBG_LEVEL==1
	#define AT_DBG(...) printf(__VA_ARGS__)
#else
	#define AT_DBG(...) 
#endif


typedef struct ATimer_t {
	char name[21];
	void (*cb)(struct ATimer_t *, void *);
	struct timespec timeout;
	int period;
	ATimer_runtype type;
	struct ATimer_t *next;
} ATimer_t;

ATimer_t *ATQ; // Pointer to the head of the list
ATimer_t *done; // Pointer to head of timers that need to be processed
struct timespec tnowTick;

pthread_cond_t wCond;
pthread_mutex_t wCondMutex = PTHREAD_MUTEX_INITIALIZER;

pthread_t t1, t2;
volatile int bye = 0;

static void *ATimer_tick_thread(void *arg)
{
	AT_DBG("ATimer_tick_thread: start\n");
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
	AT_DBG("ATimer_tick_thread: end\n");
}

static void ATimer_enqueue(ATimer_t *new)
{
	new->next = ATQ;
	// Insert at the right place
	ATimer_t *p;
	while (new->next && new->timeout.tv_sec >= new->next->timeout.tv_sec) {
		p = new->next;
		new->next = new->next->next;
	}
	if (new->next == ATQ) {
		ATQ = new;
		AT_DBG("\t > Inserted @ head\n");
		return;
	}
	p->next = new;
	AT_DBG("\t > Inserted after %s\n", p->name);
}

static void *ATimer_worker_thread(void *arg)
{
	AT_DBG("ATimer_worker_thread: start\n");
	while (bye < 2) {
		pthread_mutex_lock(&wCondMutex);
		pthread_cond_wait(&wCond, &wCondMutex);
		pthread_mutex_unlock(&wCondMutex);
		while (ATQ && tnowTick.tv_sec >= ATQ->timeout.tv_sec 
				&& tnowTick.tv_nsec >= ATQ->timeout.tv_nsec) 
		{
			ATimer_t *d = ATQ;
			// This should be protected by mutex
			ATQ = ATQ->next;

			AT_DBG("Processing %s\n", d->name);
			d->cb(d, NULL);
			if (d->type == AT_ONESHOT) {
				free(d);
			} else {
				d->timeout.tv_sec += d->period;
				ATimer_enqueue(d);
			}
		}
	}
	bye = 3;
	AT_DBG("ATimer_worker_thread: end\n");
}

void ATimer_init()
{
	ATQ = done = NULL;
	pthread_create(&t1, NULL, ATimer_tick_thread, NULL);
	pthread_create(&t2, NULL, ATimer_worker_thread, NULL);
	AT_DBG("ATimer_init: ATimer initialized\n");
}

void ATimer_deinit()
{
	AT_DBG("ATimer_deinit: Shutting down ATimer threads\n");
	bye = 1;
	while (bye < 3) {
		usleep(1000);
	}
	AT_DBG("ATimer_deinit: Cleaning up ATimer Queue\n");
	while (ATQ) {
		ATimer_t *h = ATQ;
		ATQ = ATQ->next;
		free(h);
	}
	AT_DBG("ATimer_deinit: ATimer Deinitialized\n");
}

void ATimer_start(
	char *name, 
	int period, 
	ATimer_runtype type, 
	void (*cb)(ATimer_t *, void *)
	)
{
	ATimer_t *new = malloc(sizeof(ATimer_t));
	new->cb = cb;
	bzero(new->name, 21);
	strcpy(new->name, name);
	new->period = period;
	struct timespec tnow;
	clock_gettime(CLOCK_REALTIME, &tnow);
	new->timeout = tnow;
	new->timeout.tv_sec += period;
	new->type = type;
	AT_DBG("ATimer_start: %s\n", new->name);

	ATimer_enqueue(new);	
	// printQ();
}

void printQ(void)
{
	ATimer_t *h = ATQ;
	while (h) {
		AT_DBG("%s->", h->name);
		h = h->next;
	}
	AT_DBG("NULL\n");
}

char *ATimer_getname(ATimer_t *ptr)
{
	return ptr->name;
}

# Application Timer

This program simulates something like an embedded software type, soft application timer. The calling program is given a way to initialize the timer module, then start either oneshot or repeating timers and provide callbacks for them.

Sample output for the existing program (ver 0.1):
``` shell
$ ./test_atimer
ATimer_tick_thread: start
ATimer_init: ATimer initialized
ATimer_worker_thread: start
ATimer_start: timer1
         > Inserted @ head
ATimer_start: timer2
         > Inserted after timer1
Processing timer1
callback for timer timer1: fired
         > Inserted after timer2
Processing timer2
callback for timer timer2: fired
         > Inserted after timer1
Processing timer1
callback for timer timer1: fired
         > Inserted @ head
Processing timer1
callback for timer timer1: fired
         > Inserted after timer2
Processing timer2
callback for timer timer2: fired
         > Inserted after timer1
Processing timer1
callback for timer timer1: fired
         > Inserted @ head
ATimer_deinit: Shutting down ATimer threads
ATimer_tick_thread: end
ATimer_worker_thread: end
ATimer_deinit: Cleaning up ATimer Queue
ATimer_deinit: ATimer Deinitialized
Tested
```

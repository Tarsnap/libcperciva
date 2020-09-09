#include <pthread.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>

#include "warnp.h"

#include "send_alarms.h"

struct send_alarms {
	pthread_t to_thr;
	volatile int * stop;
};

static void *
workthread_send_alarm(void * cookie)
{
	struct send_alarms * SA = cookie;

	while (*SA->stop == 0) {
		if (pthread_kill(SA->to_thr, SIGALRM)) {
			warnp("pthread_kill");
			goto err0;
		}
	}

	return (NULL);

err0:
	warn0("Failed!");
	return (NULL);
}

/*
 * start_alarms(to_thr, stop, thr):
 * Start a thread, stored in ${thr}, which repeatedly sends SIGALRM to
 * ${to_thr}.  Stop when ${stop} is non-zero.
 */
int
start_alarms(pthread_t to_thr, volatile int * stop, pthread_t * thr)
{
	struct send_alarms * SA;
	int rc;

	/* Allocate structure. */
	if ((SA = malloc(sizeof(struct send_alarms))) == NULL)
		goto err0;
	SA->to_thr = to_thr;
	SA->stop = stop;

	/* Create thread. */
	if ((rc = pthread_create(thr, NULL, workthread_send_alarm, SA)) != 0) {
		warn0("pthread_create: %s", strerror(rc));
		goto err1;
	}

	/* Success! */
	return (0);

err1:
	free(SA);
err0:
	/* Failure! */
	return(1);
}

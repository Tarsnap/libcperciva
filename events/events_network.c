#include <poll.h>

#include <errno.h>
#include <stdlib.h>

#include "elasticarray.h"
#include "warnp.h"

#include "events.h"
#include "events_internal.h"

/* Structure for holding readability and writability events for a socket. */
struct socketrec {
	struct eventrec * reader;
	struct eventrec * writer;
};

/* List of sockets. */
ELASTICARRAY_DECL(SOCKETLIST, socketlist, struct socketrec);
static SOCKETLIST S = NULL;

/* File descriptors to be polled. */
static struct pollfd * fds = NULL;
static nfds_t nfds;

/* Position to which events_network_get has scanned in *fds. */
static nfds_t fdscanpos;

/* Number of registered events. */
static size_t nev;

/* Initialize the socket list if we haven't already done so. */
static int
initsocketlist(void)
{

	/* If we're already initialized, do nothing. */
	if (S != NULL)
		goto done;

	/* Initialize the socket list. */
	if ((S = socketlist_init(0)) == NULL)
		goto err0;

	/* There are no events registered. */
	nev = 0;

	/* There are no unevented ready sockets. */
	fdscanpos = 0;

done:
	/* Success! */
	return (0);

err0:
	/* Failure! */
	return (-1);
}

/* Grow the socket list and initialize new records. */
static int
growsocketlist(size_t nrec)
{
	size_t i;

	/* Get the old size. */
	i = socketlist_getsize(S);

	/* Grow the list. */
	if (socketlist_resize(S, nrec))
		goto err0;

	/* Initialize new members. */
	for (; i < nrec; i++) {
		socketlist_get(S, i)->reader = NULL;
		socketlist_get(S, i)->writer = NULL;
	}

	/* Success! */
	return (0);

err0:
	/* Failure! */
	return (-1);
}

/**
 * events_network_register(func, cookie, s, op):
 * Register ${func}(${cookie}) to be run when socket ${s} is ready for
 * reading or writing depending on whether ${op} is EVENTS_NETWORK_OP_READ or
 * EVENTS_NETWORK_OP_WRITE.  If there is already an event registration for
 * this ${s}/${op} pair, errno will be set to EEXIST and the function will
 * fail.
 */
int
events_network_register(int (*func)(void *), void * cookie, int s, int op)
{
	struct eventrec ** r;

	/* Initialize if necessary. */
	if (initsocketlist())
		goto err0;

	/* Sanity-check socket number. */
	if (s < 0) {
		warn0("Invalid file descriptor for network event: %d", s);
		goto err0;
	}

	/* Sanity-check operation. */
	if ((op != EVENTS_NETWORK_OP_READ) &&
	    (op != EVENTS_NETWORK_OP_WRITE)) {
		warn0("Invalid operation for network event: %d", op);
		goto err0;
	}

	/* Grow the array if necessary. */
	if (((size_t)(s) >= socketlist_getsize(S)) &&
	    (growsocketlist((size_t)s + 1) != 0))
		goto err0;

	/* Look up the relevant event pointer. */
	if (op == EVENTS_NETWORK_OP_READ)
		r = &socketlist_get(S, (size_t)s)->reader;
	else
		r = &socketlist_get(S, (size_t)s)->writer;

	/* Error out if we already have an event registered. */
	if (*r != NULL) {
		errno = EEXIST;
		goto err0;
	}

	/* Register the new event. */
	if ((*r = events_mkrec(func, cookie)) == NULL)
		goto err0;

	/*
	 * There is a new socket/operation pair:
	 * Rebuild the fd list on the next call to events_network_select().
	 */
	if (fds != NULL) {
		free(fds);
		fds = NULL;
	}

	/*
	 * Increment events-registered counter; and if it was zero, start the
	 * inter-select duration clock.
	 */
	if (nev++ == 0)
		events_network_selectstats_startclock();

	/* Success! */
	return (0);

err0:
	/* Failure! */
	return (-1);
}

/**
 * events_network_cancel(s, op):
 * Cancel the event registered for the socket/operation pair ${s}/${op}.  If
 * there is no such registration, errno will be set to ENOENT and the
 * function will fail.
 */
int
events_network_cancel(int s, int op)
{
	struct eventrec ** r;

	/* Initialize if necessary. */
	if (initsocketlist())
		goto err0;

	/* Sanity-check socket number. */
	if (s < 0) {
		warn0("Invalid file descriptor for network event: %d", s);
		goto err0;
	}

	/* Sanity-check operation. */
	if ((op != EVENTS_NETWORK_OP_READ) &&
	    (op != EVENTS_NETWORK_OP_WRITE)) {
		warn0("Invalid operation for network event: %d", op);
		goto err0;
	}

	/* We have no events registered beyond the end of the array. */
	if ((size_t)(s) >= socketlist_getsize(S)) {
		errno = ENOENT;
		goto err0;
	}

	/* Look up the relevant event pointer. */
	if (op == EVENTS_NETWORK_OP_READ)
		r = &socketlist_get(S, (size_t)s)->reader;
	else
		r = &socketlist_get(S, (size_t)s)->writer;

	/* Check if we have an event. */
	if (*r == NULL) {
		errno = ENOENT;
		goto err0;
	}

	/* Free the event. */
	events_freerec(*r);
	*r = NULL;

	/*
	 * Since there is no longer an event registered for this socket /
	 * operation pair, it doesn't make any sense for it to be ready:
	 * Rebuild the fd list on the next call to events_network_select().
	 */
	if (fds != NULL) {
		free(fds);
		fds = NULL;
	}

	/*
	 * Decrement events-registered counter; and if it is becoming zero,
	 * stop the inter-select duration clock.
	 */
	if (--nev == 0)
		events_network_selectstats_stopclock();

	/* Success! */
	return (0);

err0:
	/* Failure! */
	return (-1);
}

/**
 * events_network_select(tv):
 * Check for socket readiness events, waiting up to ${tv} time if there are
 * no sockets immediately ready, or indefinitely if ${tv} is NULL.  The value
 * stored in ${tv} may be modified.
 */
int
events_network_select(struct timeval * tv)
{
	size_t i;

	/* Initialize if necessary. */
	if (initsocketlist())
		goto err0;

	/* Allocate and fill fds list. */
	if (fds == NULL) {
		if ((fds = calloc(socketlist_getsize(S),
		    sizeof(struct pollfd))) == NULL) {
			warnp("calloc()");
			goto err0;
		}
		nfds = 0;

		/* ... and add the ones we care about. */
		for (i = 0; i < socketlist_getsize(S); i++) {
			if (socketlist_get(S, i)->reader ||
			    socketlist_get(S, i)->writer) {
				fds[nfds].fd = i;
				if (socketlist_get(S, i)->reader)
					fds[nfds].events |= POLLIN;
				if (socketlist_get(S, i)->writer)
					fds[nfds].events |= POLLOUT;
				nfds++;
			}
		}
	}

	/* We're about to call poll! */
	events_network_selectstats_select();

	/* Poll. */
	while (poll(fds, nfds, (tv == NULL) ?
	    -1 : (tv->tv_sec * 1000 + (int) (tv->tv_usec * 0.001))) == -1) {
		/* EINTR is harmless. */
		if (errno == EINTR)
			continue;

		/* Anything else is an error. */
		warnp("poll()");
		goto err0;
	}

	/* If we have any events registered, start the clock again. */
	if (nev > 0)
		events_network_selectstats_startclock();

	/* We should start scanning for events at the beginning. */
	fdscanpos = 0;

	/* Success! */
	return (0);

err0:
	/* Failure! */
	return (-1);
}

/**
 * events_network_get(void):
 * Find a socket readiness event which was identified by a previous call to
 * events_network_select, and return it as an eventrec structure; or return
 * NULL if there are no such events available.  The caller is responsible for
 * freeing the returned memory.
 */
struct eventrec *
events_network_get(void)
{
	struct eventrec * r;

	/* We haven't found any events yet. */
	r = NULL;

	/* We have no fds, thus we have no events. */
	if (fds == NULL)
		return (r);

	/* Scan through the fd sets looking for ready sockets. */
	for (; fdscanpos < nfds; fdscanpos++) {
		struct pollfd * current = &fds[fdscanpos];

		/* File descriptor was closed, ignore it in future polls. */
		if (current->revents & POLLNVAL)
			current->fd = -1;

		/* Are we ready for reading? */
		if (current->revents & POLLIN) {
			r = socketlist_get(S, current->fd)->reader;
			socketlist_get(S, current->fd)->reader = NULL;
			if (--nev == 0)
				events_network_selectstats_stopclock();

			/* Remove POLLIN from events and revents. */
			current->events = current->events & ~POLLIN;
			current->revents = current->revents & ~POLLIN;
			break;
		}

		/* Are we ready for writing? */
		if (current->revents & POLLOUT) {
			r = socketlist_get(S, current->fd)->writer;
			socketlist_get(S, current->fd)->writer = NULL;
			if (--nev == 0)
				events_network_selectstats_stopclock();

			/* Remove POLLOUT from events and revents. */
			current->events = current->events & ~POLLOUT;
			current->revents = current->revents & ~POLLOUT;
			break;
		}
	}

	/* Return the event we found, or NULL if we didn't find any. */
	return (r);
}

/**
 * events_network_shutdown(void)
 * Clean up and free memory.  This call is not necessary on program exit and
 * is only expected to be useful when checking for memory leaks.
 */
void
events_network_shutdown(void)
{

	/* If we're not initialized, do nothing. */
	if (S == NULL)
		return;

	/* If we have any registered events, do nothing. */
	if (nev > 0)
		return;

	/* Free the fds list. */
	if (fds != NULL) {
		free(fds);
		fds = NULL;
	}

	/* Free the socket list. */
	socketlist_free(S);
	S = NULL;
}

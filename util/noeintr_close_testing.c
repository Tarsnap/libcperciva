#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "noeintr_close_testing.h"

/*
 * We're not guarding against race conditions for these variables;
 * the point is merely that after the test is finished, they're non-zero,
 * and approximately equal.
 */
static size_t count_normal = 0;
static size_t count_eintr_noclosed = 0;
static size_t count_eintr_closed = 0;

/**
 * noeintr_close_testing_evil_close(fd):
 * POSIX is silent about the state of ${fd} if close(2) fails with errno set
 * to EINTR.  For internal testing, this function randomly selects among the
 * behaviours which are permitted by the standard.  For more info, see:
 * https://www.daemonology.net/blog/2011-12-17-POSIX-close-is-broken.html
 * https://www.austingroupbugs.net/view.php?id=529
 */
int
noeintr_close_testing_evil_close(int fd)
{

	/* This random value is not used for any cryptographic purpose. */
	switch (random() % 3) {
	case 0:
		count_normal++;

		/* Normal close(). */
		return (close(fd));
	case 1:
		count_eintr_noclosed++;

		/*
		 * Fail with EINTR, without changing the open file
		 * description to which fd refers.
		 */
		errno = EINTR;
		return (-1);
	case 2:
		count_eintr_closed++;

		/* Normal close(), then fail with EINTR. */
		close(fd);
		errno = EINTR;
		return (-1);
	}

	/* UNREACHABLE */
	abort();
}

/**
 * noeintr_close_testing_print_stats(void):
 * Print results of testing an implementation of close() which randomly
 * 1) calls close() normally, 2) reports EINTR without calling close(), or
 * 3) calls close() but then reports EINTR.
 */
void
noeintr_close_testing_print_stats(void)
{

	printf("noeintr_close_testing_evil_close()'s stats:\t%zu\t%zu\t%zu\n",
	    count_normal, count_eintr_noclosed, count_eintr_closed);
}

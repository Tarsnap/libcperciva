#ifndef NOEINTR_CLOSE_TESTING_H_
#define NOEINTR_CLOSE_TESTING_H_

/**
 * noeintr_close_testing_evil_close(fd):
 * POSIX is silent about the state of ${fd} if close(2) fails with errno set
 * to EINTR.  For internal testing, this function randomly selects among the
 * behaviours which are permitted by the standard.  For more info, see:
 * https://www.daemonology.net/blog/2011-12-17-POSIX-close-is-broken.html
 * https://www.austingroupbugs.net/view.php?id=529
 */
int noeintr_close_testing_evil_close(int);

/**
 * noeintr_close_testing_print_stats(void):
 * Print results of testing an implementation of close() which randomly
 * 1) calls close() normally, 2) reports EINTR without calling close(), or
 * 3) calls close() but then reports EINTR.
 */
void noeintr_close_testing_print_stats(void);

#endif /* !NOEINTR_CLOSE_TESTING_H_ */

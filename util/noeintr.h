#ifndef NOEINTR_H_
#define NOEINTR_H_

#include <unistd.h>

/**
 * noeintr_write(d, buf, nbytes):
 * Write ${nbytes} bytes of data from ${buf} to the file descriptor ${d} per
 * the write(2) system call, but looping until completion if interrupted by
 * a signal.  Return ${nbytes} on success or -1 on error.
 */
ssize_t noeintr_write(int, const void *, size_t);

/**
 * noeintr_close(fd):
 * Close the file descriptor ${fd} per the close(2) system call, but retry on
 * EINTR if the descriptor was not closed.  Unlike close(2), this function is
 * not async-signal-safe.
 */
int noeintr_close(int);

#endif /* !NOEINTR_H_ */

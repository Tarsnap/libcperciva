#ifndef _POSIX_CLOSE_H_
#define _POSIX_CLOSE_H_

/*
 * Use a #define in order to avoid namespace collisions with anyone else's
 * posix_close() (e.g., musl).
 */
#define posix_close libcperciva_posix_close

/**
 * posix_close(fd, flags):
 * Close a descriptor, but looping until completion if interrupted by a signal.
 * ${flags} must be 0.
 */
int posix_close(int, int);

#endif /* _POSIX_CLOSE_H_ */

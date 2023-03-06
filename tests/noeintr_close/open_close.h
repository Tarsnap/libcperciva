#ifndef OPEN_CLOSE_H_
#define OPEN_CLOSE_H_

/**
 * open_close(filename, nbytes):
 * Open the ${filename}, read ${bytes} (which must be 0 or 1), then close it.
 */
int open_close(const char *, int);

#endif /* !OPEN_CLOSE_H_ */

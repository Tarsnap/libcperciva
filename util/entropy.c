#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <unistd.h>

#include "apisupport.h"
#include "warnp.h"

#include "entropy.h"

#ifdef APISUPPORT_LINUX_GETRANDOM
#include <sys/random.h>
#endif

/**
 * XXX Portability
 * XXX We obtain random bytes from the operating system by opening
 * XXX /dev/urandom and reading them from that device; this works on
 * XXX modern UNIX-like operating systems but not on systems like
 * XXX win32 where there is no concept of /dev/urandom.
 */

/**
 * entropy_read(buf, buflen):
 * Fill the given buffer with random bytes provided by the operating system.
 */
int
entropy_read(uint8_t * buf, size_t buflen)
{
	int fd;
	ssize_t lenread;

	/* Sanity-check the buffer size. */
	if (buflen > SSIZE_MAX) {
		warn0("Programmer error: "
		    "Trying to read insane amount of random data: %zu",
		    buflen);
		goto err0;
	}

#ifdef APISUPPORT_LINUX_GETRANDOM
	if (apisupport_linux_getrandom()) {
		if (getrandom(buf, buflen, 0) != (ssize_t)buflen) {
			warn0("getrandom()");
			goto err0;
		}
		return (0);
	}
#endif

	/* Open /dev/urandom. */
	if ((fd = open("/dev/urandom", O_RDONLY)) == -1) {
		warnp("open(/dev/urandom)");
		goto err0;
	}

	/* Read bytes until we have filled the buffer. */
	while (buflen > 0) {
		if ((lenread = read(fd, buf, buflen)) == -1) {
			warnp("read(/dev/urandom)");
			goto err1;
		}

		/* The random device should never EOF. */
		if (lenread == 0) {
			warn0("EOF on /dev/urandom?");
			goto err1;
		}

		/* We've filled a portion of the buffer. */
		buf += (size_t)lenread;
		buflen -= (size_t)lenread;
	}

	/* Close the device. */
	while (close(fd) == -1) {
		if (errno != EINTR) {
			warnp("close(/dev/urandom)");
			goto err0;
		}
	}

	/* Success! */
	return (0);

err1:
	close(fd);
err0:
	/* Failure! */
	return (-1);
}

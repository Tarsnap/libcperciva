#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "warn.h"

#include "readpass.h"

#define MAXPASSLEN 2048

/**
 * tarsnap_readpass(passwd, prompt, confirmprompt, devtty)
 * If ${devtty} is non-zero, read a password from /dev/tty if possible; if
 * not, read from stdin.  If reading from a tty (either /dev/tty or stdin),
 * disable echo and prompt the user by printing ${prompt} to stderr.  If
 * ${confirmprompt} is non-NULL, read a second password (prompting if a
 * terminal is being used) and repeat until the user enters the same password
 * twice.  Return the password as a malloced NUL-terminated string via
 * ${passwd}.  The obscure name is to avoid namespace collisions due to the
 * getpass / readpass / readpassphrase / etc. functions in various libraries.
 */
int
tarsnap_readpass(char ** passwd, const char * prompt,
    const char * confirmprompt, int devtty)
{
	FILE * readfrom;
	char passbuf[MAXPASSLEN];
	char confpassbuf[MAXPASSLEN];
	struct termios term, term_old;
	int usingtty;

	/*
	 * If devtty != 0, try to open /dev/tty; if that fails, or if devtty
	 * is zero, we'll read the password from stdin instead.
	 */
	if ((devtty == 0) || ((readfrom = fopen("/dev/tty", "r")) == NULL))
		readfrom = stdin;

	/* If we're reading from a terminal, try to disable echo. */
	if ((usingtty = isatty(fileno(readfrom))) != 0) {
		if (tcgetattr(fileno(readfrom), &term_old)) {
			warn("Cannot read terminal settings");
			goto err1;
		}
		memcpy(&term, &term_old, sizeof(struct termios));
		term.c_lflag = (term.c_lflag & ~ECHO) | ECHONL;
		if (tcsetattr(fileno(readfrom), TCSANOW, &term)) {
			warn("Cannot set terminal settings");
			goto err1;
		}
	}

retry:
	/* If we have a terminal, prompt the user to enter the password. */
	if (usingtty)
		fprintf(stderr, "%s: ", prompt);

	/* Read the password. */
	if (fgets(passbuf, MAXPASSLEN, readfrom) == NULL) {
		warn("Cannot read password");
		goto err2;
	}

	/* Confirm the password if necessary. */
	if (confirmprompt != NULL) {
		if (usingtty)
			fprintf(stderr, "%s: ", confirmprompt);
		if (fgets(confpassbuf, MAXPASSLEN, readfrom) == NULL) {
			warn("Cannot read password");
			goto err2;
		}
		if (strcmp(passbuf, confpassbuf)) {
			fprintf(stderr,
			    "Passwords mismatch, please try again\n");
			goto retry;
		}
	}

	/* Terminate the string at the first "\r" or "\n" (if any). */
	passbuf[strcspn(passbuf, "\r\n")] = '\0';

	/* If we changed terminal settings, reset them. */
	if (usingtty)
		tcsetattr(fileno(readfrom), TCSANOW, &term_old);

	/* Close /dev/tty if we opened it. */
	if (readfrom != stdin)
		fclose(readfrom);

	/* Copy the password out. */
	if ((*passwd = strdup(passbuf)) == NULL) {
		warn("Cannot allocate memory");
		goto err1;
	}

	/* Zero any stored passwords. */
	memset(passbuf, 0, MAXPASSLEN);
	memset(confpassbuf, 0, MAXPASSLEN);

	/* Success! */
	return (0);

err2:
	/* Reset terminal settings if necessary. */
	if (usingtty)
		tcsetattr(fileno(readfrom), TCSAFLUSH, &term_old);
err1:
	/* Close /dev/tty if we opened it. */
	if (readfrom != stdin)
		fclose(readfrom);

	/* Failure! */
	return (-1);
}

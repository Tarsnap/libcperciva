#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "warnp.h"

#include "setuidgid.h"

/* <sysexits.h> isn't in POSIX, so we define this manually. */
#define EX_NOPERM	77	/* permission denied */

int
main(int argc, char * argv[])
{
	const char * opt_username_groupname;

	WARNP_INIT;

	/* Print usage, if necessary. */
	if (argc != 2) {
		fprintf(stderr, "usage: test_setuid "
		    "{USERNAME | :GROUPNAME | USERNAME:GROUPNAME}\n");
		goto err0;
	}
	opt_username_groupname = argv[1];

	/* Change username (and groupname). */
	if (setuidgid(opt_username_groupname, SETUIDGID_SGROUP_LEAVE_WARN)) {
		if (errno == EPERM) {
			warn0("Insufficient permissions to change your user"
			    " or group(s).");
			goto nopermission;
		}
		goto err0;
	}

	/* Success! */
	return (0);

nopermission:
	/* Failure! */
	exit(EX_NOPERM);

err0:
	/* Failure! */
	exit(1);
}

#include <sys/types.h>

#include <assert.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "parsenum.h"
#include "setgroups_none.h"
#include "warnp.h"

#include "setuidgid.h"

/* Function prototypes related to supplementary groups. */
static int check_supplementary_groups_none(void);

/* Function prototypes related to uid and gid. */
static int set_group(const char *);
static int set_user(const char *);
static int string_extract_user_group(const char *, char **, char **);

/* Check if we're in any supplementary groups. */
static int
check_supplementary_groups_none(void)
{
	gid_t grouplist[1];
	int ngroups;

	/* Find number of groups. */
	if ((ngroups = getgroups(0, NULL)) < 0) {
		warnp("getgroups()");
		goto err0;
	}

	/* Check group membership. */
	if (ngroups > 1)
		/* Definitely failed. */
		goto err0;
	if (ngroups == 1) {
		/*
		 * POSIX allows getgroups() to return the effective group ID,
		 * so if we're in exactly one group, we need to check that GID.
		 */
		if (getgroups(1, grouplist) != 1) {
			warnp("getgroups()");
			goto err0;
		}
		/* POSIX: getegid() shall not modify errno. */
		if (grouplist[0] != getegid())
			goto err0;
	}

	/* Success! */
	return (0);

err0:
	/* Failure! */
	return (-1);
}

/* Set the GID to the group represented by ${groupname}. */
static int
set_group(const char * groupname)
{
	gid_t gid;
	struct group * group_info;

	/*
	 * Attempt to convert the group name to a group ID.  If the database
	 * lookup fails with an error, return; but if it fails without an error
	 * (indicating that it successfully found that the name does not exist)
	 * fall back to trying to parse the name as a numeric group ID.
	 */
	errno = 0;
	if ((group_info = getgrnam(groupname)) != NULL) {
		gid = group_info->gr_gid;
	} else if (errno) {
		warnp("getgrnam(%s)", groupname);
		goto err0;
	} else if (PARSENUM(&gid, groupname)) {
		warn0("No such group: %s", groupname);
		goto err0;
	}

	/* Set GID. */
	if (setgid(gid)) {
		/* We handle EPERM separately; keep it in errno. */
		if (errno != EPERM)
			warnp("setgid(%lu)", (unsigned long int)gid);
		goto err0;
	}

	/* Success! */
	return (0);

err0:
	/* Failure! */
	return (-1);
}

/* Set the UID to the user represented by ${username}. */
static int
set_user(const char * username)
{
	uid_t uid;
	struct passwd * user_info;

	/* See similar code in set_gid(). */
	errno = 0;
	if ((user_info = getpwnam(username)) != NULL) {
		uid = user_info->pw_uid;
	} else if (errno) {
		warnp("getpwnam(%s)", username);
		goto err0;
	} else if (PARSENUM(&uid, username)) {
		warn0("No such user: %s", username);
		goto err0;
	}

	/* Set UID. */
	if (setuid(uid)) {
		/* We handle EPERM separately; keep it in errno. */
		if (errno != EPERM)
			warnp("setuid(%lu)", (unsigned long int)uid);
		goto err0;
	}

	/* Success! */
	return (0);

err0:
	/* Failure! */
	return (-1);
}

/* Parses "username", ":groupname", or "username:groupname". */
static int
string_extract_user_group(const char * combined, char ** username_p,
    char ** groupname_p)
{
	const char * s;
	size_t username_len;

	/* Sanity check. */
	assert(combined != NULL);

	/* If there's a ':', what follows is the group name. */
	if ((s = strchr(combined, ':')) != NULL) {
		/* Duplicate the group name. */
		if ((*groupname_p = strdup(&s[1])) == NULL) {
			warnp("strdup");
			goto err0;
		}
	} else {
		/* No group name. */
		*groupname_p = NULL;

		/* User name includes everything prior to terminating NUL. */
		s = &combined[strlen(combined)];
	}

	/* Anything prior to the ':' or terminating NUL is the user name. */
	if (s > combined) {
		username_len = (size_t)(s - combined);

		/* Duplicate the user name. */
		if ((*username_p = malloc(username_len + 1)) == NULL) {
			warnp("malloc");
			goto err1;
		}
		memcpy(*username_p, combined, username_len);
		(*username_p)[username_len] = '\0';
	} else {
		/* No user name. */
		*username_p = NULL;
	}

	/* Reject empty group names. */
	if ((*groupname_p != NULL) && (strlen(*groupname_p) == 0)) {
		warn0("Empty group name: \"%s\"", combined);
		goto err2;
	}

	/* Reject strings with neither user nor group. */
	if ((*groupname_p == NULL) && (*username_p == NULL)) {
		warn0("Need to specify user and/or group: \"%s\"", combined);
		goto err2;
	}

	/* Success! */
	return (0);

err2:
	free(*username_p);
err1:
	free(*groupname_p);
err0:
	/* Failure! */
	return (-1);
}

/**
 * setuidgid(user_group_string, leave_suppgrp):
 * Set the UID and/or GID to the names given in ${user_group_string}.  If no
 * UID or GID can be found matching those strings, treat the values as numeric
 * IDs.  Depending on the existence and position of a colon ":", the behaviour
 * is
 * - no ":" means that the string is a username.
 * - ":" in the first position means that the string is a groupname.
 * - otherwise, the string is parsed into "username:groupname".
 *
 * The behaviour with supplementary groups depends on ${leave_suppgrp}:
 * - SETUIDGID_SGROUP_IGNORE: do not attempt to leave supplementary groups.
 * - SETUIDGID_SGROUP_LEAVE_WARN: attempt to leave; if it fails, give a
 *   warning but continue.
 * - SETUIDGID_SGROUP_LEAVE_ERROR: attempt to leave; if it fails, return
 *   an error.
 */
int
setuidgid(const char * user_group_string, int leave_suppgrp)
{
	char * username;
	char * groupname;
	int saved_errno;

	/* Get the username:groupname from ${user_group_string}. */
	if (string_extract_user_group(user_group_string, &username, &groupname))
		goto err0;

	/* If requested, leave supplementary groups. */
	if (leave_suppgrp != SETUIDGID_SGROUP_IGNORE) {
		/* Attempt to leave all supplementary groups. */
		if (setgroups_none()) {
			if (errno != EPERM) {
				warnp("setgroups()");
				goto err1;
			}

			/* We handle EPERM separately; keep it in errno. */
			if (leave_suppgrp == SETUIDGID_SGROUP_LEAVE_ERROR)
				goto err1;
		}

		/* Check that we have actually left all supplementary groups. */
		if (check_supplementary_groups_none()) {
			if (leave_suppgrp == SETUIDGID_SGROUP_LEAVE_ERROR) {
				warn0("Failed to leave supplementary groups");
				goto err1;
			}

			/* Print warning, but don't indicate an error. */
			warn0("Warning: Failed to leave supplementary groups");
		}
	}

	/* Set group; must be changed before user. */
	if (groupname && set_group(groupname))
		goto err1;

	/* Set user. */
	if (username && set_user(username))
		goto err1;

	/* Clean up. */
	free(groupname);
	free(username);

	/* Success! */
	return (0);

err1:
	/* POSIX Issue 7 does not forbid free() from modifying errno. */
	saved_errno = errno;

	/* Clean up. */
	free(groupname);
	free(username);

	/* Restore errno. */
	errno = saved_errno;
err0:
	/* Failure! */
	return (-1);
}

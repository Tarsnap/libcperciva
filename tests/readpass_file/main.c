#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "insecure_memzero.h"
#include "readpass.h"
#include "warnp.h"

int
main(int argc, char * argv[])
{
	const char * filename;
	char * passwd;

	WARNP_INIT;

	/* Process arguments. */
	if (argc != 2) {
		fprintf(stderr, "usage: tests_readpass_file FILENAME\n");
		goto err0;
	}
	filename = argv[1];

	/* Read passphrase. */
	if (readpass_file(&passwd, filename))
		goto err0;

	/* Print passphrase. */
	printf("%s\n", passwd);

	/* Clean up. */
	insecure_memzero(passwd, strlen(passwd));
	free(passwd);

	/* Success! */
	exit(0);

err0:
	/* Failure! */
	exit(1);
}

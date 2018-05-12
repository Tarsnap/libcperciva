#include <stdio.h>
#include <stdlib.h>

#include "warnp.h"

#include "crypto_entropy.h"

/* This must match the value in crypto/crypto_entropy.c! */
#define RESEED_INTERVAL 256

int
main(int argc, char * argv[])
{
	uint8_t buf[32];
	size_t i;

	WARNP_INIT;

	(void)argc;
	(void)argv;

	/* Get entropy enough times to trigger a reseed. */
	for (i = 0; i < RESEED_INTERVAL + 1; i++) {
		if (crypto_entropy_read(buf, 32)) {
			warn0("crypto_entropy_read(): internal error.");
			goto err0;
		}
	}

	/* Print final random value. */
	for (i = 0; i < 32; i++) {
		printf("%02x", buf[i]);
	}
	printf("\n");

	/* Success! */
	exit(0);

err0:
	/* Failure! */
	exit(1);
}

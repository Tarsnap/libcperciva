#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "warnp.h"

#include "aws_readkeys.h"
#include "aws_sign.h"

static int
s3_headers(const char * key_id, const char * key_secret)
{
	const char * region = "reg";
	const char * method = "method";
	const char * bucket = "bucket";
	const char * path = "path";
	const char * body = "body";
	char * amz_content;
	char * amz_date;
	char * auth;

	/* Generate a header. */
	if (aws_sign_s3_headers(key_id, key_secret, region, method, bucket,
	    path, (const uint8_t *)body, strlen(body), &amz_content,
	    &amz_date, &auth)) {
		warn0("aws_sign_s3_headers");
		goto err0;
	}

	/* This computed value doesn't change. */
	fprintf(stderr, "content:\t%s\n", amz_content);

	/* The date and auth change, so they're not great for testing. */

	/* Clean up. */
	free(amz_content);
	free(amz_date);
	free(auth);

	/* Success! */
	return (0);

err0:
	/* Failure! */
	return (-1);
}

int
main(int argc, char ** argv)
{
	const char * filename_bad = "/dev/null";
	const char * filename_good = "format.key";
	char * key_id;
	char * key_secret;

	WARNP_INIT;
	(void)argc; /* UNUSED */

	/* Generate a failure. */
	fprintf(stderr, "---- Trying to find AWS keys in %s\n", filename_bad);
	if (!aws_readkeys(filename_bad, &key_id, &key_secret)) {
		warn0("That should have failed!");
		goto err0;
	}

	/* Read key info from real file. */
	fprintf(stderr, "---- Trying to find AWS keys in %s\n", filename_good);
	if (aws_readkeys(filename_good, &key_id, &key_secret)) {
		warn0("aws_readkeys");
		goto err0;
	}

	/* Display key info. */
	fprintf(stderr, "id:\t%s\n", key_id);
	fprintf(stderr, "secret:\t%s\n", key_secret);

	/* Generate a header. */
	fprintf(stderr, "---- Generate a header\n");
	if (s3_headers(key_id, key_secret))
		goto err0;

	/* Clean up */
	free(key_id);
	free(key_secret);

	/* Success! */
	exit(0);

err0:
	/* Failure! */
	exit(1);
}

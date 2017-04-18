#ifndef _RFC3986_H_
#define _RFC3986_H_

/**
 * rfc3986_encode(s):
 * URL-encode the provided string by replacing any characters which are not
 * in [a-zA-Z0-9~._-] with %XX equivalents.
 */
char * rfc3986_encode(const char *);

#endif /* !_RFC3986_H_ */

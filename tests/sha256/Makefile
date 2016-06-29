.POSIX:

all:	test_sha256

test:	test_sha256
	@./test_sha256 -x

LDADD_POSIX=`export CC="${CC}"; cd ../../POSIX && command -p sh posix-l.sh "$$PATH"`
CFLAGS_POSIX=`export CC="${CC}"; cd ../../POSIX && command -p sh posix-cflags.sh "$$PATH"`

test_sha256:
	${MAKE} -f Makefile.inc1 CFLAGS_POSIX="${CFLAGS_POSIX}" LDADD_POSIX="${LDADD_POSIX}" CFLAGS="${CFLAGS}" test_sha256

clean:
	${MAKE} -f Makefile.inc1 clean

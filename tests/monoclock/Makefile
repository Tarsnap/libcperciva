.POSIX:

all:	test_monoclock

test:	test_monoclock
	@./test_monoclock

LDADD_POSIX=`export CC="${CC}"; cd ../../POSIX && command -p sh posix-l.sh "$$PATH"`
CFLAGS_POSIX=`export CC="${CC}"; cd ../../POSIX && command -p sh posix-cflags.sh "$$PATH"`

test_monoclock:
	${MAKE} -f Makefile.inc1 CFLAGS_POSIX="${CFLAGS_POSIX}" LDADD_POSIX="${LDADD_POSIX}" CFLAGS="${CFLAGS}" test_monoclock

clean:
	${MAKE} -f Makefile.inc1 clean

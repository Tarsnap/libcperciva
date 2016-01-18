.POSIX:

all:	buildall

test:	all
	@true

LDADD_POSIX=`export CC="${CC}"; cd ../../POSIX && command -p sh posix-l.sh`
CFLAGS_POSIX=`export CC="${CC}"; cd ../../POSIX && command -p sh posix-cflags.sh`

buildall:
	${MAKE} -f Makefile.inc1 CFLAGS_POSIX="${CFLAGS_POSIX}" LDADD_POSIX="${LDADD_POSIX}" buildall

clean:
	${MAKE} -f Makefile.inc1 clean

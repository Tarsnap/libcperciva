.POSIX:

SUBDIR=	crc32 heap getopt sha256 humansize parsenum buildall buildsingles

all:
	for D in ${SUBDIR}; do ${MAKE} -C $${D} all || exit 1; done

test:
	for D in ${SUBDIR}; do ${MAKE} -C $${D} test || exit 1; done

clean:
	for D in ${SUBDIR}; do ${MAKE} -C $${D} clean || exit 1; done

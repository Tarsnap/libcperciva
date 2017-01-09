.POSIX:

SUBDIR=	crc32 heap getopt sha256 humansize buildall buildsingles

all:
	for D in ${SUBDIR}; do ( cd $${D} && make all ); done

test:
	for D in ${SUBDIR}; do ( cd $${D} && make test ); done

clean:
	for D in ${SUBDIR}; do ( cd $${D} && make clean ); done

.POSIX:
# AUTOGENERATED FILE, DO NOT EDIT
PROG=test_buildsingles
SRCS=main.c
IDIRS=-I../../alg -I../../aws -I../../cpusupport -I../../crypto -I../../datastruct -I../../events -I../../external/queue -I../../http -I../../netbuf -I../../network -I../../network_ssl -I../../util
SUBDIR_DEPTH=../..
RELATIVE_DIR=tests/buildsingles
LIBALL=../../liball/liball.a ../../liball/optional_mutex_normal/liball_optional_mutex_normal.a

all:
	if [ -z "$${HAVE_BUILD_FLAGS}" ]; then \
		cd ${SUBDIR_DEPTH}; \
		${MAKE} BUILD_SUBDIR=${RELATIVE_DIR} \
		    BUILD_TARGET=${PROG} buildsubdir; \
	else \
		${MAKE} ${PROG} all_extra; \
	fi

clean:	clean_extra
	rm -f ${PROG} ${SRCS:.c=.o}

${PROG}:${SRCS:.c=.o} ${LIBALL}
	${CC} -o ${PROG} ${SRCS:.c=.o} ${LIBALL} ${LDFLAGS} ${LDADD_EXTRA} ${LDADD_REQ} ${LDADD_POSIX}

main.o: main.c
	${CC} ${CFLAGS_POSIX} -D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE=700 -DCPUSUPPORT_CONFIG_FILE=\"cpusupport-config.h\" -DAPISUPPORT_CONFIG_FILE=\"apisupport-config.h\" -I../.. ${IDIRS} ${CPPFLAGS} ${CFLAGS} -c main.c -o main.o

test:
	true

all_extra:
	${MAKE} -f Makefile-headers CFLAGS_POSIX="${CFLAGS_POSIX}" CPPFLAGS="${CPPFLAGS}" CFLAGS="${CFLAGS}" IDIRS="${IDIRS}" buildsingles

clean_extra:
	${MAKE} -f Makefile-headers clean

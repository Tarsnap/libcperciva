.POSIX:
# AUTOGENERATED FILE, DO NOT EDIT
PROG=test_optional_mutex_pthread
SRCS=main.c
IDIRS=-I../../../util
LDADD_REQ=-lpthread
SUBDIR_DEPTH=../../..
RELATIVE_DIR=tests/optional_mutex/pthread
LIBALL=../../../liball/liball.a ../../../liball/optional_mutex_pthread/liball_optional_mutex_pthread.a

all:
	if [ -z "$${HAVE_BUILD_FLAGS}" ]; then \
		cd ${SUBDIR_DEPTH}; \
		${MAKE} BUILD_SUBDIR=${RELATIVE_DIR} \
		    BUILD_TARGET=${PROG} buildsubdir; \
	else \
		${MAKE} ${PROG}; \
	fi

clean:
	rm -f ${PROG} ${SRCS:.c=.o}

${PROG}:${SRCS:.c=.o} ${LIBALL}
	${CC} -o ${PROG} ${SRCS:.c=.o} ${LIBALL} ${LDFLAGS} ${LDADD_EXTRA} ${LDADD_REQ} ${LDADD_POSIX}

main.o: ../main.c ../../../util/millisleep.h ../../../util/optional_mutex.h ../../../util/warnp.h
	${CC} ${CFLAGS_POSIX} -D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE=700 -DCPUSUPPORT_CONFIG_FILE=\"cpusupport-config.h\" -DAPISUPPORT_CONFIG_FILE=\"apisupport-config.h\" -I../../.. ${IDIRS} ${CPPFLAGS} ${CFLAGS} -DNUM_THREADS=10 -c ../main.c -o main.o

test:	all
	./${PROG}

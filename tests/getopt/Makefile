.POSIX:

all:	test_getopt

test:	all
	@printf "Testing command-line options parsing... "
	@/bin/sh -e test_getopt_args.sh 2>test_getopt.log || true
	@if cmp -s test_getopt.log test_getopt.good; then	\
		echo "PASSED!";					\
	else							\
		echo "FAILED!";					\
		echo "test_getopt.log:";			\
		cat test_getopt.log;				\
	fi

test_getopt:	main.o getopt.o
	${CC} ${CFLAGS} -o test_getopt main.o getopt.o

main.o:	main.c
	${CC} ${CFLAGS} -D_POSIX_C_SOURCE=200809L -I ../../util -c main.c -o main.o

getopt.o:	../../util/getopt.c
	${CC} ${CFLAGS} -D_POSIX_C_SOURCE=200809L -I ../../util -c ../../util/getopt.c -o getopt.o

clean:
	rm -f test_getopt
	rm -f main.o getopt.o
	rm -f test_getopt.log


.POSIX:

all:	test_getopt

test:	all
	@./test_getopt.sh

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


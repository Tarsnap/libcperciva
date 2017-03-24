.POSIX:

all:	test_parsenum

test:	test_parsenum
	@./test_parsenum

test_parsenum:	main.o warnp.o
	${CC} ${CFLAGS} -lm -o test_parsenum main.o warnp.o

main.o:	main.c ../../util/parsenum.h
	${CC} -D_POSIX_C_SOURCE=200809L ${CFLAGS} -I ../../util -c main.c -o main.o

warnp.o:	../../util/warnp.c
	${CC} -D_POSIX_C_SOURCE=200809L ${CFLAGS} -I ../../util -c ../../util/warnp.c -o warnp.o

clean:
	rm -f test_parsenum
	rm -f main.o warnp.o
	rm -f test_parsenum.log

.POSIX:

all:	test_parsenum

test:	all
	@printf "Testing PARSENUM... "
	@./test_parsenum 2>test_parsenum.log || true
	@if cmp -s test_parsenum.log test_parsenum.good; then		\
		echo "PASSED!";						\
	else								\
		echo "FAILED!";						\
		echo "test_parsenum.log:";				\
		cat test_parsenum.log;					\
	fi

test_parsenum:	main.o warnp.o
	${CC} ${CFLAGS} -o test_parsenum main.o warnp.o

main.o:	main.c
	${CC} -D_POSIX_C_SOURCE=200809L ${CFLAGS} -I ../../util -c main.c -o main.o

warnp.o:	../../util/warnp.c
	${CC} -D_POSIX_C_SOURCE=200809L ${CFLAGS} -I ../../util -c ../../util/warnp.c -o warnp.o

clean:
	rm -f test_parsenum
	rm -f main.o warnp.o
	rm -f test_parsenum.log

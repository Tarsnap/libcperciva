.POSIX:

all:	test_humansize

test:	test_humansize
	@./test_humansize

test_humansize:	main.o humansize.o asprintf.o warnp.o
	${CC} -o test_humansize main.o humansize.o asprintf.o warnp.o

main.o:	main.c
	${CC} -D_POSIX_C_SOURCE=200809L -I ../../util -c main.c -o main.o

humansize.o:	../../util/humansize.c
	${CC} -D_POSIX_C_SOURCE=200809L -I ../../util -c ../../util/humansize.c -o humansize.o

asprintf.o:	../../util/asprintf.c
	${CC} -D_POSIX_C_SOURCE=200809L -I ../../util -c ../../util/asprintf.c -o asprintf.o

warnp.o:	../../util/warnp.c
	${CC} -D_POSIX_C_SOURCE=200809L -I ../../util -c ../../util/warnp.c -o warnp.o

clean:
	rm -f test_humansize
	rm -f main.o humansize.o asprintf.o warnp.o

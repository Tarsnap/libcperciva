.POSIX:

all:	test_sha256

test:	test_sha256
	@./test_sha256 -x

test_sha256:	main.o sha256.o getopt.o hexify.o insecure_memzero.o warnp.o
	${CC} -o test_sha256 main.o sha256.o getopt.o hexify.o insecure_memzero.o warnp.o

main.o:	main.c
	${CC} -D_POSIX_C_SOURCE=200809L -D__BSD_VISIBLE -I ../../alg -I ../../util -c main.c -o main.o

sha256.o:	../../alg/sha256.c
	${CC} -D_POSIX_C_SOURCE=200809L -std=c99 -I ../../alg -I ../../util -c ../../alg/sha256.c -o sha256.o

getopt.o:	../../util/getopt.c
	${CC} -D_POSIX_C_SOURCE=200809L -I ../../util -c ../../util/getopt.c -o getopt.o

hexify.o:	../../util/hexify.c
	${CC} -D_POSIX_C_SOURCE=200809L -I ../../util -c ../../util/hexify.c -o hexify.o

insecure_memzero.o:	../../util/insecure_memzero.c
	${CC} -D_POSIX_C_SOURCE=200809L -I ../../util -c ../../util/insecure_memzero.c -o insecure_memzero.o

warnp.o:	../../util/warnp.c
	${CC} -D_POSIX_C_SOURCE=200809L -I ../../util -c ../../util/warnp.c -o warnp.o

clean:
	rm -f test_sha256
	rm -f main.o sha256.o getopt.o hexify.o insecure_memzero.o warnp.o

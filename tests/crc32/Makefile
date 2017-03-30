.POSIX:

all:	test_crc32

test:	test_crc32
	@./test_crc32

test_crc32:	main.o crc32c.o
	${CC} ${CFLAGS} -o test_crc32 main.o crc32c.o

main.o:	main.c
	${CC} ${CFLAGS} -D_POSIX_C_SOURCE=200809L -I ../../alg -c main.c -o main.o

crc32c.o:	../../alg/crc32c.c
	${CC} ${CFLAGS} -D_POSIX_C_SOURCE=200809L -I ../../alg -c ../../alg/crc32c.c -o crc32c.o

clean:
	rm -f test_crc32
	rm -f main.o crc32c.o

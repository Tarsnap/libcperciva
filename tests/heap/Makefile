.POSIX:

all:	test_heap

test:	test_heap
	@./test_heap.sh

test_heap: main.o elasticarray.o ptrheap.o warnp.o
	${CC} ${CFLAGS} -o test_heap main.o elasticarray.o ptrheap.o warnp.o

main.o: main.c
	${CC} ${CFLAGS} -D_POSIX_C_SOURCE=200809L -I ../../datastruct -I ../../util -c main.c -o main.o

elasticarray.o: ../../datastruct/elasticarray.c
	${CC} ${CFLAGS} -D_POSIX_C_SOURCE=200809L -I ../../datastruct -I ../../util -c ../../datastruct/elasticarray.c -o elasticarray.o

ptrheap.o: ../../datastruct/ptrheap.c
	${CC} ${CFLAGS} -D_POSIX_C_SOURCE=200809L -I ../../datastruct -I ../../util -c ../../datastruct/ptrheap.c -o ptrheap.o

warnp.o: ../../util/warnp.c
	${CC} ${CFLAGS} -D_POSIX_C_SOURCE=200809L -I ../../datastruct -I ../../util -c ../../util/warnp.c -o warnp.o

clean:
	rm -f test_heap
	rm -f main.o elasticarray.o ptrheap.o warnp.o

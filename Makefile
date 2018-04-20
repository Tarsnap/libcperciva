.POSIX:

PKG=	libcperciva
PROGS=
TESTS=	tests/buildall tests/buildsingles tests/crc32 tests/getopt tests/heap \
	tests/humansize tests/monoclock tests/mpool tests/parsenum \
	tests/setuidgid tests/sha256 tests/valgrind
PUBLISH= ${PROGS} COPYRIGHT STYLE POSIX alg cpusupport crypto datastruct \
	events network tests util
BINDIR_DEFAULT=	/usr/local/bin
CFLAGS_DEFAULT=	-O2
LIBCPERCIVA_DIR=	.

all: cpusupport-config.h
	export CFLAGS="$${CFLAGS:-${CFLAGS_DEFAULT}}";	\
	export "LDADD_POSIX=`export CC=\"${CC}\"; cd ${LIBCPERCIVA_DIR}/POSIX && command -p sh posix-l.sh \"$$PATH\"`";	\
	export "CFLAGS_POSIX=`export CC=\"${CC}\"; cd ${LIBCPERCIVA_DIR}/POSIX && command -p sh posix-cflags.sh \"$$PATH\"`";	\
	. ./cpusupport-config.h;			\
	for D in ${PROGS} ${TESTS}; do			\
		( cd $${D} && ${MAKE} all ) || exit 2;	\
	done

cpusupport-config.h:
	( export CC="${CC}"; command -p sh ${LIBCPERCIVA_DIR}/cpusupport/Build/cpusupport.sh "$$PATH" ) > cpusupport-config.h

install: all
	export BINDIR=$${BINDIR:-${BINDIR_DEFAULT}};	\
	for D in ${PROGS}; do				\
		( cd $${D} && ${MAKE} install ) || exit 2;	\
	done

clean:
	rm -f cpusupport-config.h
	for D in ${PROGS} ${TESTS}; do				\
		( cd $${D} && ${MAKE} clean ) || exit 2;	\
	done

.PHONY: test
test:	all
	tests/test_libcperciva.sh

# Developer targets: These only work with BSD make
Makefiles:
	${MAKE} -f Makefile.BSD Makefiles

publish:
	${MAKE} -f Makefile.BSD publish

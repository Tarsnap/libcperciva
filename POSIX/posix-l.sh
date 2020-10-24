# Should be sourced by `command -p sh posix-l.sh "$PATH"` from within a Makefile.

# Sanity check environment variables
if [ -z "${CC}" ]; then
	echo "\$CC is not defined!  Cannot run any compiler tests." 1>&2
	exit 1
fi
if ! [ ${PATH} = "$1" ]; then
	echo "WARNING: POSIX violation: $SHELL's command -p resets \$PATH" 1>&2
	PATH=$1
fi

# Find directory of this script and the source files
D=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd -P)

FIRST=YES
for LIB in rt xnet; do
	if ${CC} ${CFLAGS} -l${LIB} posix-l.c 2>/dev/null; then
		if [ ${FIRST} = "NO" ]; then
			printf " ";
		fi
		printf "%s" "-l${LIB}";
		FIRST=NO;
	else
		echo "WARNING: POSIX violation: ${CC} does not understand -l${LIB}" 1>&2
	fi
	rm -f a.out
done

${CC} ${CFLAGS} $D/posix-strtod.c 2>/dev/null
if ! ./a.out ; then
	echo "WARNING: C99 violation: ${CC} does not accept hex strings in strtod" 1>&2
	${CC} ${CFLAGS} -std=c99 $D/posix-strtod.c 2>/dev/null
	if ./a.out ; then
		[ ${FIRST} = "NO" ] && printf " "; FIRST=NO
		printf %s "-std=c99"
	fi
fi

rm -f ./a.out

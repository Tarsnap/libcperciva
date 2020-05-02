# Should be sourced by
#     command -p sh posix-l.sh "$PATH" "$CC"
# from within a Makefile.

# Process & sanity check environment variables.
if [ "$#" -ne 2 ]; then
	echo "Incorrect number of arguments." 1>&2
	exit 1
fi
CC=$2
if [ -z "${CC}" ]; then
	echo "\$CC is not defined!  Cannot run any compiler tests." 1>&2
	exit 1
fi
if ! [ ${PATH} = "$1" ]; then
	echo "WARNING: POSIX violation: $SHELL's command -p resets \$PATH" 1>&2
	PATH=$1
fi

FIRST=YES
for LIB in rt xnet; do
	if ${CC} -l${LIB} posix-l.c 2>/dev/null; then
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

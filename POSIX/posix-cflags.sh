# Should be sourced by `command -p sh posix-cflags.sh "$PATH"` from within a Makefile

# Sanity check environment variables
if [ -z "${CC}" ]; then
	echo "\$CC is not defined!  Cannot run any compiler tests." 1>&2
	exit 1
fi
if ! [ ${PATH} = "$1" ]; then
	echo "WARNING: POSIX violation: $SHELL's command -p resets \$PATH" 1>&2
	PATH=$1
fi

FIRST=YES
if ! ${CC} -D_POSIX_C_SOURCE=200809L posix-msg_nosignal.c 2>/dev/null; then
	[ ${FIRST} = "NO" ] && printf " "; FIRST=NO
	printf %s "-DPOSIXFAIL_MSG_NOSIGNAL"
	echo "WARNING: POSIX violation: <sys/socket.h> not defining MSG_NOSIGNAL" 1>&2
fi
if ! ${CC} -D_POSIX_C_SOURCE=200809L posix-clock_realtime.c 2>/dev/null; then
	[ ${FIRST} = "NO" ] && printf " "; FIRST=NO
	printf %s "-DPOSIXFAIL_CLOCK_REALTIME"
	echo "WARNING: POSIX violation: <time.h> not defining CLOCK_REALTIME" 1>&2
fi
rm -f posix-clock_gettime
if ! ${CC} -D_POSIX_C_SOURCE=200809L -o posix-clock_gettime posix-clock_gettime.c 2>/dev/null; then
	[ ${FIRST} = "NO" ] && printf " "; FIRST=NO
	printf %s "-DPOSIXFAIL_CLOCK_GETTIME"
	echo "WARNING: POSIX violation: <time.h> not declaring clock_gettime()" 1>&2
else
	# We need to run the binary, because OSX 10.11 (and below) uses dynamic
	# linking to a library which doesn't include clock_gettime().
	# Annoyingly, there's two levels of error output: one from the binary
	# itself, and one from the signal it sends to the calling process.
	# The "$( ./x 2>y ) 2>y" captures both types of error message.
	if ! $( ./posix-clock_gettime 2>/dev/null ) 2>/dev/null ; then
		[ ${FIRST} = "NO" ] && printf " "; FIRST=NO
		printf %s "-DPOSIXFAIL_CLOCK_GETTIME"
		echo "WARNING: POSIX violation: clock_gettime() is not linkable" 1>&2
	fi
fi
rm -f posix-clock_gettime
if ! ${CC} -D_POSIX_C_SOURCE=200809L posix-restrict.c 2>/dev/null; then
	echo "WARNING: POSIX violation: ${CC} does not accept 'restrict' keyword" 1>&2
	if ${CC} -D_POSIX_C_SOURCE=200809L -std=c99 posix-restrict.c 2>/dev/null; then
		[ ${FIRST} = "NO" ] && printf " "; FIRST=NO
		printf %s "-std=c99"
	fi
fi
rm -f a.out

#!/bin/sh

set -e -o nounset

D=$1
MAKEBSD=$2
CFLAGS_HARDCODED=$3

OUT=Makefile

# Check environment
if [ -z "$CPP" ]; then
	echo "Need CPP environment variable."
	exit 1
fi

# Check command-line arguments
if [ "$#" -ne 3 ]; then
	echo "Usage: $0 DIR MAKEBSD CFLAGS_HARDCODED"
	exit 1
fi

# Set up directories
cd "${D}"
SUBDIR_DEPTH=$(${MAKEBSD} -v SUBDIR_DEPTH)
LIBCPERCIVA_DIR=$(${MAKEBSD} -v LIBCPERCIVA_DIR)

# Set up *-config.h so that we don't have missing headers.  If we don't
# have a LIBCPERCIVA_DIR, then we assume that we don't have cpusupport and
# apisupport.
if [ -n "${LIBCPERCIVA_DIR}" ]; then
	if [ -e "${LIBCPERCIVA_DIR}/cpusupport/Build/cpusupport.sh" ]; then
		command -p sh						\
		    "${LIBCPERCIVA_DIR}/cpusupport/Build/cpusupport.sh"	\
		    "${PATH}" --all > "${SUBDIR_DEPTH}/cpusupport-config.h"
	fi
	if [ -e "${LIBCPERCIVA_DIR}/apisupport/Build/apisupport.sh" ]; then
		command -p sh						\
		    "${LIBCPERCIVA_DIR}/apisupport/Build/apisupport.sh"	\
		    "${PATH}" --all > "${SUBDIR_DEPTH}/apisupport-config.h"
	fi
fi

copyvar() {
	var=$1
	if [ -n "$(${MAKEBSD} -v "$var")" ]; then
		printf "%s=" "$var" >> $OUT
		${MAKEBSD} -v "$var" >> $OUT
	fi
}

addvar_lib() {
	var=$1
	varval=$(${MAKEBSD} -v "$var")
	if [ -n "${varval}" ]; then
		printf "%s=lib%s.a\n" "$var" "${varval}" >> $OUT
	fi
}

add_makefile_prog() {
	# Get a copy of the default Makefile.prog
	cp "${SUBDIR_DEPTH}/release-tools/Makefile.prog" prog.tmp

	# Remove the "install:" (if applicable)
	if [ "$(${MAKEBSD} -v NOINST)" = "1" ]; then
		perl -0pe 's/(install:.*?)\n\n//s' prog.tmp > prog.tmp1
		mv prog.tmp1 prog.tmp
	fi

	# Remove " ${LIBALL}" (if applicable)
	if [ "$(${MAKEBSD} -v NOLIBALL)" = "1" ]; then
		perl -0pe 's/ \$\{LIBALL\}//g' prog.tmp > prog.tmp1
		mv prog.tmp1 prog.tmp
	fi

	# Add the (adjusted) Makefile.prog to the Makefile, and clean up
	cat prog.tmp >> $OUT
	rm prog.tmp
}

get_cpusupport_cflags() {
	src=$1

	str=$(grep 'CPUSUPPORT CFLAGS:' "${src}" | cut -f 2- -d :)
	# ${str} must be unquoted.
	for X in ${str}; do
		printf " \${CFLAGS_%s}" "$X"
	done | sed 's/^ //'
}

get_apisupport_cflags() {
	src=$1

	str=$(grep 'APISUPPORT CFLAGS:' "${src}" | cut -f 2- -d :)
	# ${str} must be unquoted.
	for X in ${str}; do
		printf " \${CFLAGS_%s}" "$X"
	done | sed 's/^ //'
}

add_object_files() {
	# Set up useful variables
	OBJ=$(${MAKEBSD} -v SRCS |				\
	    sed -e 's| apisupport-config.h||' |			\
	    sed -e 's| cpusupport-config.h||' |			\
	    tr ' ' '\n' |					\
	    sed -E 's/.c$/.o/' )
	CPP_CONFIG="-DCPUSUPPORT_CONFIG_FILE=\"cpusupport-config.h\" -DAPISUPPORT_CONFIG_FILE=\"apisupport-config.h\""
	CPP_ARGS_FIXED="-std=c99 ${CPP_CONFIG} -I${SUBDIR_DEPTH} -MM"
	OUT_CC_BEGIN="\${CC} \${CFLAGS_POSIX} ${CFLAGS_HARDCODED}"
	OUT_CC_MID="-I${SUBDIR_DEPTH} \${IDIRS} \${CPPFLAGS} \${CFLAGS}"

	# Generate build instructions for each object
	for F in $OBJ; do
		S=$(${MAKEBSD} source-"${F}")
		CF_MANUAL=$(${MAKEBSD} -v CFLAGS."$(basename "${S}")")
		CF_CPUSUPPORT=$(get_cpusupport_cflags "${S}")
		CF_APISUPPORT=$(get_apisupport_cflags "${S}")
		CF=$(echo "${CF_CPUSUPPORT} ${CF_APISUPPORT} ${CF_MANUAL}" | \
		    sed 's/^ //' | sed 's/ $//')
		IDIRS=$(${MAKEBSD} -v IDIRS)
		# Get the build dependencies, then remove newlines, condense
		# multiple spaces, remove line continuations, and replace the
		# final space with a newline.
		${CPP} "${S}" ${CPP_ARGS_FIXED} ${CF_MANUAL} ${IDIRS}	\
		    -MT "${F}" |	\
		    tr '\n' ' ' |					\
		    tr -s ' '	|					\
		    sed -e 's| \\ | |g' |				\
		    sed -e 's| $||g'
		printf "\n"
		# Print the build instructions
		echo "	${OUT_CC_BEGIN} ${OUT_CC_MID} ${CF} -c ${S} -o ${F}" |
		    tr -s ' '
	done >> $OUT
}

# Add header and variables
echo '.POSIX:' > $OUT
echo '# AUTOGENERATED FILE, DO NOT EDIT' >> $OUT
if [ -n "$(${MAKEBSD} -v LIB)" ]; then
	addvar_lib LIB
else
	copyvar PROG
fi
copyvar MAN1

# SRCS is trickier to handle, as we need to remove any -config.h from the list.
if [ -n "$(${MAKEBSD} -v SRCS)" ]; then
	printf "SRCS=" >> $OUT
	${MAKEBSD} -v SRCS |				\
	    sed -e 's| apisupport-config.h||' |		\
	    sed -e 's| cpusupport-config.h||' >> $OUT
fi
copyvar IDIRS
copyvar LDADD_REQ
copyvar SUBDIR_DEPTH
printf "RELATIVE_DIR=%s\n" "$D" >> $OUT

# Add all, install, clean, $PROG
if [ -n "$(${MAKEBSD} -v LIB)" ]; then
	cat "${SUBDIR_DEPTH}/release-tools/Makefile.lib" >> $OUT
elif [ -n "$(${MAKEBSD} -v SRCS)" ]; then
	copyvar LIBALL
	add_makefile_prog
else
	printf "\nall:\n\ttrue\n\nclean:\n\ttrue\n" >> $OUT
fi

# Add all object files (if applicable)
if [ -n "$(${MAKEBSD} -v SRCS)" ]; then
	add_object_files
fi

# Add test (if applicable)
if grep -q "^test:" Makefile.BSD ; then
	printf "\n" >> $OUT
	awk '/^test:/, /^$/' Makefile.BSD |			\
	    awk '$1' >> $OUT
fi

# Add perftest (if applicable)
if grep -q "^perftest:" Makefile.BSD ; then
	printf "\n" >> $OUT
	awk '/^perftest:/, /^$/' Makefile.BSD |			\
	    awk '$1' >> $OUT
fi

# Add all_extra (if applicable)
if grep -q "^all_extra:" Makefile.BSD ; then
	printf "\n" >> $OUT
	awk '/^all_extra:/, /^$/' Makefile.BSD |		\
	    awk '$1' >> $OUT
	sed -e 's/${MAKE} ${PROG}/${MAKE} ${PROG} all_extra/'	\
	    Makefile > Makefile.new
	mv Makefile.new Makefile
fi

# Add clean_extra (if applicable)
if grep -q "^clean_extra:" Makefile.BSD ; then
	printf "\n" >> $OUT
	awk '/^clean_extra:/, /^$/' Makefile.BSD |		\
	    awk '$1' >> $OUT
	awk '/^clean:/ {print $0 "\tclean_extra";next}{print}'	\
	    Makefile > Makefile.new
	mv Makefile.new Makefile
fi

# Clean up -config.h files
rm -f "${SUBDIR_DEPTH}/cpusupport-config.h"
rm -f "${SUBDIR_DEPTH}/apisupport-config.h"

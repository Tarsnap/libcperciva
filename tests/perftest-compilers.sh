#!/bin/sh

# Run perftest-generate.sh in the current directory with (potentially)
# multiple compilers, varying $CFLAGS.  Store the results in an
# easily-plottable file.

# The list of CFLAGS is inside the forall() function.

set -e -o nounset

# Constant (potentially) influenced by the environment.
CFLAGS_ORIG="${CFLAGS:-}"

# Dirname-related constant.
scriptdir=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd -P)

# Parse command line.
dirname=""
tag=""
use_count="0"
use_git_hash="0"
while getopts "cd:ght:" opt
do
	case ${opt} in
		c)	use_count="1" ;;
		d)	dirname="${OPTARG}" ;;
		g)	use_git_hash="1" ;;
		t)	tag="${OPTARG}" ;;
		h | *)	printf "usage: $0 [-c] [-d dirname] [-g] [-t tag]\n"
			printf "       compiler1 compiler2 [...]\n"
			exit 1 ;;
	esac
done
shift "$((OPTIND - 1))"
if [ "$#" -eq "0" ]; then
	printf "usage: $0 [-c] [-d dirname] [-g] [-t tag]\n"
	printf "       compiler1 compiler2 [...]\n"
	exit 1
fi
compilers=$*

# Set default dirname.
if [ -z "${dirname}" ]; then
	dirname="$(pwd)"
fi
basename="$(basename "${dirname}")"

# Check for (expected) common mistakes.
if [ "${basename}" = "tests" ] || [ "${basename}" = "libcperciva" ]; then
	printf "Error: $0 should be invoked from a tests/ subdirectory\n"
	exit 1
fi

# Generate output for a (${CC}, ${CFLAGS}) pair.
generate_output() {
	compiler=$1
	oflag=$2
	usetag=$3

	# Set up compiler environment.
	export CC="${compiler}"
	if [ -n "${CFLAGS_ORIG}" ]; then
		export CFLAGS="${CFLAGS_ORIG} ${oflag}"
	else
		export CFLAGS="${oflag}"
	fi

	# Compilers may require different flags for cpusupport.
	rm -f "${scriptdir}"/../cpusupport-config.h

	# Extra args.
	extra_args="-d ${dirname}"
	if [ "${use_count}" -gt 0 ]; then
		extra_args="${extra_args} -c"
	fi
	if [ "${use_git_hash}" -gt 0 ]; then
		extra_args="${extra_args} -g"
	fi

	# Run perftest.  Don't double-quote ${extra_args} or ${usetag}.
	"${scriptdir}"/perftest-generate.sh ${extra_args} -t ${usetag}
}

# Find the highest speeds for each (${CC}, ${CFLAGS}) pair out of the
# multiple tests.
extract_highest_speeds() {
	# Ignore $1 and $2; they are not relevant here.
	usetag=$3
	perftest_basename="perftest-${basename}-${usetag}"
	output="${perftest_basename}-highest.txt"

	# Find buffer sizes.
	bufsizes="$(grep -v "#" "${perftest_basename}.txt" |	\
	    cut -f 1 | sort -n | uniq)"

	# Find highest speeds & save to a file.
	for bufsize in ${bufsizes}; do
		highest="$(awk "/^${bufsize}\t/" "${perftest_basename}.txt" | \
		    cut -f 2 | sort -rn | head -1)"
		printf "${bufsize}\t${highest}\n" >> "${output}"
	done
}

# Run a function on every (${CC}, ${CFLAGS}) pair.
forall() {
	func=$1

	# For every pair...
	for compiler in ${compilers}; do
		for oflag in "-O2" "-O3 -march=native"; do
			# ... format a name for this pair's output...
			cctag="$(echo				\
			    "${compiler}${oflag}" |		\
			    sed "s/ /-/" |			\
			    sed "s/=/-/g" |			\
			    sed "s/--/-/g")"
			if [ -n "${tag}" ]; then
				usetag="${tag}-${cctag}"
			else
				usetag="${cctag}"
			fi

			# ... and run the function.
			${func} "${compiler}" "${oflag}" "${usetag}"
		done
	done
}

print_highest_speeds() {
	files="$(ls -- *-highest.txt)"

	# Find buffer sizes.
	one_file="$(echo "${files}" | head -1)"
	bufsizes="$(grep -v "#" "${one_file}" | cut -f 1 | sort -n | uniq)"

	# Print header, omitting the "perftest-${basename}-" prefix
	# and the "-highest.txt" suffix.
	omit_begin=$((1 + "$(printf "perftest-${basename}-" | wc -c)" ))
	for f in ${files} ; do
		name="$(echo "${f%-highest.txt}" | cut -c "${omit_begin}-")"
		printf "\t${name}"
	done
	printf "\n"

	# Print the highest speeds for each compiler & cflags pairs.
	for bufsize in ${bufsizes}; do
		printf "${bufsize}"
		for f in ${files} ; do
			speed="$(awk "/^${bufsize}\t/" "$f" | cut -f 2)"
			printf "\t${speed}"
		done
		printf "\n"
	done
}

# Delete previous data.
rm -f "${dirname}/perftest-${basename}-*.txt"

# Main.
forall generate_output
forall extract_highest_speeds
print_highest_speeds

# Clean up intermediate data.  Use selective quotes so that the glob works.
rm -f "${dirname}"/perftest-"${basename}"-*-highest.txt

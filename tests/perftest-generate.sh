#!/bin/sh

# Run `make perftest` in the indicated directory multiple times with the
# current $CC and $CFLAGS.  Store the results in an easily-plottable file.
# The filename can optionally contain a user-specified tag, a
# directory-specific iteration count, and/or the current git hash.

set -e -o nounset

# Constant (potentially) influenced by the environment.
NUM_REPS=${NUM_REPS:-3}

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
		t)	tag="-${OPTARG}" ;;
		h | *)	printf "usage: $0 [-c] [-d dirname] [-g] [-t tag]\n"
			exit 1 ;;
	esac
done
shift "$((OPTIND - 1))"
if [ "$#" -ne "0" ]; then
	printf "usage: $0 [-d dirname] [-t tag]\n"
	exit 1
fi

# Set default dirname.
if [ -z "${dirname}" ]; then
	dirname="$(pwd)"
fi
basename="$(basename "${dirname}")"

# Check for (expected) common mistakes.
if [ "${basename}" = "tests" ] || [ "${basename}" = "libcperciva" ]; then
	printf "Error: specify a [-d dirname] or invoke from a"
	printf " tests/ subdirectory\n"
	exit 1
fi

# Append the count to the tag (if applicable).
if [ "${use_count}" -gt 0 ]; then
	# The count number is stored in this file, as a method of transferring
	# that variable between iterations of the script (if necessary).
	numfile="${dirname}/perftest-num.tmp"

	# Initialize ${count}.
	if [ -e "${numfile}" ]; then
		count="$(cat "${numfile}")"
	else
		count="0"
	fi
	countstr="$(printf "%02d" "${count}")"

	# Save next count to the file.
	printf "%02d" "$((count + 1))" > "${numfile}"

	# Update ${tag}.
	tag="${tag}-${countstr}"
fi

# Append the git hash to the tag (if applicable).
if [ "${use_git_hash}" -gt 0 ]; then
	# Are there any uncommited changes in git?
	if [ -z "$(git status --untracked-files=no --porcelain)" ]; then
		hashstr="$(git rev-parse --short HEAD)"
	else
		hashstr="nogit"
	fi

	# Update ${tag}.
	tag="${tag}-${hashstr}"
fi

# Filename variables.
rawdata="perftest-${basename}${tag}.raw"
data="perftest-${basename}${tag}.txt"

# Delete previous data.
rm -f "${data}"
rm -f "${rawdata}"

# Make sure that we're in the right directory.
cd "${dirname}"

# Compile.
printf "### make clean all\n" >> "${rawdata}"
make clean all >> "${rawdata}" 2>&1

# Generate raw data.
i=0
while [ $i -lt "${NUM_REPS}" ]; do
	printf "### ${i} make perftest\n"
	make perftest | tee -a ${data}
	i=$((i + 1))
done >> "${rawdata}" 2>&1

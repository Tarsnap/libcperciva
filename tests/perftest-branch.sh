#!/bin/sh

# Generate perftests for every commit in the current branch.

set -e -o nounset

# Dirname-related constants.
scriptdir=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd -P)

# Parse command line.
dirname=""
git_object=""
while getopts "d:ho:" opt
do
	case ${opt} in
		d)	dirname="${OPTARG}" ;;
		o)	git_object="${OPTARG}" ;;
		h | *)	printf "usage: $0 [-d dirname] [-o git_object]\n"
			exit 1 ;;
	esac
done
shift "$((OPTIND - 1))"
if [ "$#" -ne "0" ]; then
	printf "usage: $0 [-d dirname] [-o git_object]\n"
	exit 1
fi

# Set default dirname.
if [ -z "${dirname}" ]; then
	dirname="$(pwd)"
fi

# Initialize branch commit count.
numfile="${dirname}/perftest-num.tmp"
rm -f "${numfile}"

# Run script on each commit.
git rebase "${git_object}"						\
    --exec "${scriptdir}/perftest-generate.sh -c -d \"${dirname}\" -g"

# Clean up.
rm "${numfile}"

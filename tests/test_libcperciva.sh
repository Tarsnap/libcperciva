#!/bin/sh

### Find script directory and load helper functions.
scriptdir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd -P)
. ${scriptdir}/shared_test_functions.sh


### Project-specific constants and setup

test_scenarios="${scriptdir}/??-*.sh"
out="${bindir}/tests-output"
out_valgrind="${bindir}/tests-valgrind"


### Run tests using project-specific constants
run_scenarios ${test_scenarios}

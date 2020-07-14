#!/bin/sh

set -e

printf "%s\n" "--- should pass:"
./test_readpass_file good-noeol.pass
test `./test_readpass_file good-noeol.pass | wc -l` -eq "1"

./test_readpass_file good-eol.pass
test `./test_readpass_file good-eol.pass | wc -l` -eq "1"

./test_readpass_file good-eol-r.pass
test `./test_readpass_file good-eol-r.pass | wc -l` -eq "1"

printf "%s\n" "--- should pass (but will contain extra whitespace):"
./test_readpass_file good-spaces.pass
test `./test_readpass_file good-spaces.pass | wc -l` -eq "1"

./test_readpass_file good-tabs.pass
test `./test_readpass_file good-tabs.pass | wc -l` -eq "1"

printf "%s\n" "--- should pass: (but will be a blank password)"
./test_readpass_file good-empty.pass
test `./test_readpass_file good-empty.pass | wc -l` -eq "1"

./test_readpass_file good-only-newline.pass
test `./test_readpass_file good-only-newline.pass | wc -l` -eq "1"

# These lines should fail, which means that the "&& exit 1" won't be executed.
printf "%s\n" "--- should fail:"
./test_readpass_file this-file-does-not-exist && exit 1

./test_readpass_file bad-initial-newline.pass && exit 1

./test_readpass_file bad-extra-newline.pass && exit 1

./test_readpass_file bad-extra-material.pass  && exit 1

printf "\nok\n"

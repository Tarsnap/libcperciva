#!/bin/sh

set -e

printf "%s\n" "--- should pass:"
./test_readpass_file good-noeol.pass
./test_readpass_file good-eol.pass
./test_readpass_file good-eol-r.pass

printf "%s\n" "--- should pass (but will contain extra whitespace):"
./test_readpass_file good-spaces.pass
./test_readpass_file good-tabs.pass

printf "%s\n" "--- should pass: (but will be a blank password)"
./test_readpass_file good-empty.pass
./test_readpass_file good-only-newline.pass

printf "%s\n" "--- should fail:"
! ./test_readpass_file this-file-does-not-exist
if [ $? != "0" ]; then exit 1; fi
! ./test_readpass_file bad-initial-newline.pass
if [ $? != "0" ]; then exit 1; fi
! ./test_readpass_file bad-extra-newline.pass
if [ $? != "0" ]; then exit 1; fi
! ./test_readpass_file bad-extra-material.pass
if [ $? != "0" ]; then exit 1; fi

printf "\nok\n"

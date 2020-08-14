#!/bin/sh

curdir="$(pwd)"

git rebase master --exec "cd \"${curdir}\" && ./investigate.sh"
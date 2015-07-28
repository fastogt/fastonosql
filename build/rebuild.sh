#!/bin/sh
set -e

# clean and build
sh ./clean.sh $1 $2 $3
sh ./build.sh $1 $2 $3

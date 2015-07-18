#!/bin/sh
set -e

# clean and build
sh ./clean.sh $1
sh ./build.sh $1
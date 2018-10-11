#!/bin/sh

HOOKS="pre-commit"
ROOT_DIR=$(git rev-parse --show-toplevel)

for hook in $HOOKS; do
	if [ ! -f $ROOT_DIR/.git/hooks/$hook ]; then
		ln -s $ROOT_DIR/scripts/$hook $ROOT_DIR/.git/hooks/$hook
	fi
done


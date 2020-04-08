#!/bin/sh

set -e

SUBDIRS="helloworld-c helloworld-rs qsort sha256 bz2 regex-rs wrap6 basic-fp"

for dir in $SUBDIRS; do
  pushd $dir
  make "$@"
  popd
done

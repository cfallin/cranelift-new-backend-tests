#!/bin/sh

set -e

SUBDIRS="helloworld-c helloworld-rs qsort sha256 bz2 regex-rs"

for dir in $SUBDIRS; do
  pushd $dir
  make "$@"
  popd
done

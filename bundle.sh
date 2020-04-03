#!/bin/sh

set -e

rm -rf wasm-tests/
mkdir -p wasm-tests/
cp */*.wasm wasm-tests/
tar jcvf wasm-tests.tar.bz2 wasm-tests/

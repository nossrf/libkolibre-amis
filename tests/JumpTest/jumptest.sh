#!/bin/sh
set -x
set -e

if [ -x /usr/bin/gdb ]; then
  PREFIX="libtool --mode=execute gdb -x runbt --args"
fi

$PREFIX ./jumptest ../data/Mountains_skip/ncc.html

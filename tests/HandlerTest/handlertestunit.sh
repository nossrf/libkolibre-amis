#!/bin/sh

if [ -x /usr/bin/gdb ]; then
    PREFIX="libtool --mode=execute gdb --return-child-result --batch -x ${srcdir:-.}/../run --args"
fi


$PREFIX ./handlertestunit ${srcdir:-.}/../data/FireSafety/ncc.html

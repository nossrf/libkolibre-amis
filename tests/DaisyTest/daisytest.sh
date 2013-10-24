#!/bin/sh 

set -x
set -e

if [ -x /usr/bin/gdb ]; then
  PREFIX="libtool --mode=execute gdb -batch --return-child-result -x ${srcdir:-.}/run --args"
fi

$PREFIX ./daisytest ${srcdir:-.}/../data/Mountains_skip/ncc.html

if [ -x /usr/bin/gdb ]; then
  PREFIX="libtool --mode=execute gdb -batch --return-child-result -x ${srcdir:-.}/run --args"
fi

$PREFIX ./daisytest ${srcdir:-.}/../data/Theory_behind_players_kate/ncc.html

$PREFIX ./daisytest ${srcdir:-.}/../data/Chimpanzees_DAISY_3.0/Chimpanzees.opf

$PREFIX ./daisytest ${srcdir:-.}/../data/VBL20120911/speechgen.opf

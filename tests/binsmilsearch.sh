#!/bin/sh

if [ -x /usr/bin/gdb ]; then
    PREFIX="libtool --mode=execute gdb -return-child-result -x ${srcdir:-.}/run --args"
fi

$PREFIX ./binsmilsearch ${srcdir:-.}/data/Mountains_skip/ncc.html
$PREFIX ./binsmilsearch ${srcdir:-.}/data/Theory_behind_players_kate/ncc.html
$PREFIX ./binsmilsearch ${srcdir:-.}/data/Chimpanzees_DAISY_3.0/Chimpanzees.opf
$PREFIX ./binsmilsearch ${srcdir:-.}/data/VBL20120911/speechgen.opf

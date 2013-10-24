#!/bin/sh

if [ -x /usr/bin/gdb ]; then
    PREFIX="libtool --mode=execute gdb --return-child-result --batch -x ${srcdir:-.}/run --args"
fi

$PREFIX ./navpointtest ${srcdir:-.}/data/Mountains_skip/ncc.html
$PREFIX ./navpointtest ${srcdir:-.}/data/Theory_behind_players_kate/ncc.html
$PREFIX ./navpointtest ${srcdir:-.}/data/Chimpanzees_DAISY_3.0/Chimpanzees.opf
$PREFIX ./navpointtest ${srcdir:-.}/data/VBL20120911/speechgen.opf
$PREFIX ./navpointtest ${srcdir:-.}/data/FireSafety/ncc.html

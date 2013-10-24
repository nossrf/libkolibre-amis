#!/bin/sh

if [ -x /usr/bin/gdb ]; then
    PREFIX="libtool --mode=execute gdb --return-child-result --batch -x ${srcdir:-.}/run --args"
fi

$PREFIX ./playtitle ${srcdir:-.}/data/Mountains_skip/ncc.html
$PREFIX ./playtitle ${srcdir:-.}/data/Theory_behind_players_kate/ncc.html
$PREFIX ./playtitle ${srcdir:-.}/data/Chimpanzees_DAISY_3.0/Chimpanzees.opf
$PREFIX ./playtitle ${srcdir:-.}/data/VBL20120911/speechgen.opf
$PREFIX ./playtitle ${srcdir:-.}/data/FireSafety/ncc.html

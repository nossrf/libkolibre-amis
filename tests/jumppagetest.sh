#!/bin/sh

if [ -x /usr/bin/gdb ]; then
    PREFIX="libtool --mode=execute gdb -return-child-result -x ${srcdir:-.}/run --args"
fi

$PREFIX ./jumppagetest ${srcdir:-.}/data/1Brochure-DAISY-Consortium/ncc.html
$PREFIX ./jumppagetest ${srcdir:-.}/data/Mountains_skip/ncc.html
$PREFIX ./jumppagetest ${srcdir:-.}/data/Theory_behind_players_kate/ncc.html
$PREFIX ./jumppagetest ${srcdir:-.}/data/Chimpanzees_DAISY_3.0/Chimpanzees.opf
$PREFIX ./jumppagetest ${srcdir:-.}/data/VBL20120911/speechgen.opf

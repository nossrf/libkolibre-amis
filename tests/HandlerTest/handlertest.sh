#!/bin/sh

if [ -x /usr/bin/gdb ]; then
    PREFIX="libtool --mode=execute gdb --return-child-result -batch -x ${srcdir:-.}/runbt --args"
fi


$PREFIX ./handlertest ${srcdir:-.}/../data/Theory_behind_players_kate/ncc.html

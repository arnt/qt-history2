#!/bin/sh

me=$(dirname $0)
mkdir -p $me/out
(cd $me/out && ../../qlalr/qlalr --troll --no-debug --no-lines ../qxmlstream.g)

for f in $me/out/*.h; do
    n=$(basename $f)
    p4 open $me/../../src/xml/$n
    cp $f $me/../../src/xml/$n
done

p4 revert -a $me/../../src/xml/...
p4 diff -du $me/../../src/xml/...


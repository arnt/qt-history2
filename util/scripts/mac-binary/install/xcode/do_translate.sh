#!/bin/sh

mkdir -p "`dirname "$2/$1"`"
sed -e "s,\\\$VERSION_MAJOR\\\$,$VERSION_MAJOR,g" \
    -e "s,\\\$VERSION_MINOR\\\$,$VERSION_MINOR,g" \
    -e "s,\\\$VERSION_PATCH\\\$,$VERSION_PATCH,g" "$1" >"$2/$1"



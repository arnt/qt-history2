#!/bin/sh

#options parsing
BINDIR=
SRCDIR=
OUTDIR=
while [ -n "$1" ]; do
   echo "$1" | grep '^--*=*' >/dev/null 2>&1 || break
   OPTION=`echo $1 | sed 's,^--\([^=]*\)=\(.*\),\1,'`
   ARG=`echo $1 | sed 's,^--\([^=]*\)=\(.*\),\2,'`
   shift
   case "$OPTION" in
   qtsrc) SRCDIR="$ARG" ;;
   qtbin) BINDIR="$ARG" ;;
   outdir) OUTDIR="$ARG" ;;
   *) ;;
   esac
done
[ -z "$OUTDIR" ] && exit 1

#setup
FRAMEWORK_DIR="$OUTDIR/Library/Frameworks"
mkdir -p "$FRAMEWORK_DIR"

for lib in QtCore QtGui QtNetwork QtXml QtOpenGL QtSql Qt3Support; do
    if [ ! -d "$BINDIR/lib/${lib}.framework" ]; then
	echo "No framework for $lib!"
        continue
    fi
    cp -r "$BINDIR/lib/${lib}.framework" "$FRAMEWORK_DIR/"
    [ "$DO_DEBUG" = "no" ]  && find "$BINDIR/lib/${lib}.framework" -name '*_debug*' -exec rm -f {} \; >/dev/null 2>&1
    #no headers
    find "$FRAMEWORK_DIR/${lib}.framework/" -name Headers -exec rm -rf {} \; >/dev/null 2>&1
done

exit 0

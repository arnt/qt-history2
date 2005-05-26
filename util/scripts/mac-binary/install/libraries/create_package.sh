#!/bin/sh

#options parsing
BINDIR=
SRCDIR=
OUTDIR=
DO_DEBUG=yes
while [ -n "$1" ]; do
   echo "$1" | grep '^--*=*' >/dev/null 2>&1 || break
   OPTION=`echo $1 | sed 's,^--\([^=]*\)=\(.*\),\1,'`
   ARG=`echo $1 | sed 's,^--\([^=]*\)=\(.*\),\2,'`
   shift
   case "$OPTION" in
   debug) DO_DEBUG="$ARG" ;;
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
    cp -R "$BINDIR/lib/${lib}.framework" "$FRAMEWORK_DIR/" >/dev/null 2>&1
    ./fix_config_paths.pl "$FRAMEWORK_DIR/${lib}.framework/$lib" "$FRAMEWORK_DIR/${lib}.framework/${lib}.fixed" 
    mv "$FRAMEWORK_DIR/${lib}.framework/${lib}.fixed" "$FRAMEWORK_DIR/${lib}.framework/$lib" 
    if [ "$DO_DEBUG" = "no" ]; then
	find "$BINDIR/lib/${lib}.framework" -name '*_debug*' -exec rm -f {} \; >/dev/null 2>&1
    elif [ -e "$FRAMEWORK_DIR/${lib}.framework/${lib}_debug" ]; then
	./fix_config_paths.pl "$FRAMEWORK_DIR/${lib}.framework/${lib}_debug" "$FRAMEWORK_DIR/${lib}.framework/${lib}_debug.fixed" 
	mv "$FRAMEWORK_DIR/${lib}.framework/${lib}_debug.fixed" "$FRAMEWORK_DIR/${lib}.framework/${lib}_debug" 
    fi
    #no headers
    find "$FRAMEWORK_DIR/${lib}.framework/" -name Headers -exec rm -rf {} \; >/dev/null 2>&1
done

exit 0

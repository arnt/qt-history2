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

for lib in QtCore QtGui QtNetwork QtXml QtOpenGL QtSql Qt3Support QtSvg; do
    if [ ! -d "$BINDIR/lib/${lib}.framework" ]; then
	echo "No framework for $lib!"
        continue
    fi
    cp -R "$BINDIR/lib/${lib}.framework" "$FRAMEWORK_DIR/" >/dev/null 2>&1
    ../libraries/fix_config_paths.pl "$FRAMEWORK_DIR/${lib}.framework/Versions/${VERSION_MAJOR}.0/${lib}_debug" "$FRAMEWORK_DIR/${lib}.framework/Versions/${VERSION_MAJOR}.0/${lib}_debug.fixed" 
    mv "$FRAMEWORK_DIR/${lib}.framework/Versions/${VERSION_MAJOR}.0/${lib}_debug.fixed" "$FRAMEWORK_DIR/${lib}.framework/Versions/${VERSION_MAJOR}.0/${lib}_debug" 
    ../libraries/fix_prl_paths.pl "$FRAMEWORK_DIR/${lib}.framework/${lib}_debug.prl" "$FRAMEWORK_DIR/${lib}.framework/${lib}_debug.prl.fixed" 
    mv "$FRAMEWORK_DIR/${lib}.framework/${lib}_debug.prl.fixed" "$FRAMEWORK_DIR/${lib}.framework/${lib}_debug.prl" 
    # Remove the normal libraries and headers (they are part of another package)
    # some find command here.
    find "$FRAMEWORK_DIR/${lib}.framework" -name ${lib} -o -name ${lib}'.*' -a ! -name ${lib}.framework | xargs rm >/dev/null 2>&1
    find "$FRAMEWORK_DIR/${lib}.framework/" -name Headers -exec rm -rf {} \; >/dev/null 2>&1
done

exit 0

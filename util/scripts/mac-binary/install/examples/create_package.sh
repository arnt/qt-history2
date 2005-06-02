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

mkdir -p "$OUTDIR/Developer/Examples/Qt"

get_sources()
{
  DIR="$1"
  for file in `find $DIR -type f`; do
    case $file in #only need sources
    */ui_*.h|*/moc_*.cpp|*/qrc_*.cpp|*.app|*.app/*|*/.obj/*|*/.ui/*|*/.rcc/*|*/.moc/*|*/Makefile|*.build*|*.xcode*|*/build|*/build/*|*.doc|*~|*.gch|*.gch/*|*/.DS_Store|*/.gdb_history)  continue ;;
    *) ;;
    esac
    [ -x "$file" ] && continue
    FILE=`echo "$file" | sed "s,^$DIR,,"`
    mkdir -p "$OUTDIR/Developer/Examples/Qt/`dirname $FILE`"
    cp "$file" "$OUTDIR/Developer/Examples/Qt/$FILE"
  done
}

#examples
get_sources "$SRCDIR/examples"
if [ -d "$BINDIR/bin/qtdemo.app" ]; then
   mkdir -p "$OUTDIR/Developer/Applications/Qt/"
   cp -r "$BINDIR/bin/qtdemo.app" "$OUTDIR/Developer/Applications/Qt/"
fi
rm -rf "$OUTDIR/Developer/Examples/Qt/activeqt"

#demos
get_sources "$SRCDIR/demos" "Demos"
for demo_dir in $BINDIR/demos/*; do
    [ -d "$demo_dir" ] || continue
    demo=`basename $demo_dir`
    EXE=
    if [ -x "${demo_dir}/${demo}" ]; then
        mkdir -p "${OUTDIR}/Developer/Examples/Qt/Demos/${demo}"
	EXE="${OUTDIR}/Developer/Examples/Qt/Demos/${demo_dir}/${demo}"
        cp "${demo}/${demo}" "$EXE"
    elif [ -d "${demo_dir}/${demo}.app" ]; then #in a bundle
        mkdir -p "${OUTDIR}/Developer/Examples/Qt/Demos/${demo}"
        cp -R "${demo_dir}/${demo}.app" "${OUTDIR}/Developer/Examples/Qt/Demos/${demo}"
        EXE="${OUTDIR}/Developer/Examples/Qt/Demos/${demo}/${demo}.app/Contents/MacOS/$demo"
    fi
    if [ -x "$EXE" ]; then
	../libraries/fix_config_paths.pl "$EXE" "/tmp/tmp.exe"
	cp "/tmp/tmp.exe" "$EXE"
	rm -f "/tmp/tmp.exe"
	chmod a+x "$EXE" 
    fi
done

exit 0

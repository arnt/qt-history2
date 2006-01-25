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

copyHeader()
{
   HEADER_FILE=$1
   HEADER_DIR=$2 
   mkdir -p $HEADER_DIR
    if [ `wc -l $HEADER_FILE | awk '{print $1;}'` = "1" ]; then
       LINE=`head -1 $HEADER_FILE`
       if echo "$LINE" | grep '^#include "../../' >/dev/null 2>&1; then
           HEADER_FILE=`dirname $HEADER_FILE`/`echo $LINE | sed "s,^#include \"\([^\"]*\)\"\$,\1,"`
       fi
    fi
    cp "$HEADER_FILE" "$HEADER_DIR"
}

copyHeaderDir()
{
    BASEDIR=$1
    DESTDIR=$2
    mkdir -p "$DESTDIR"
    for header in `find "$BASEDIR" -type f`; do
        case $header in
        *_pch.h|*_p.h|*headers.pri)
            continue
            ;;
        *)
            copyHeader "$header" "$DESTDIR"
            ;;
        esac
    done
}

[ -z "$OUTDIR" ] && exit 1

#copy tools
for a in qmake uic uic3 rcc moc assistant Linguist Designer qt3to4 rccdump lrelease lupdate findtr; do
    EXE=
    if [ -d "${BINDIR}/bin/${a}.app" ]; then
	mkdir -p "$OUTDIR/Developer/Applications/Qt/"
	cp -R "${BINDIR}/bin/${a}.app" "$OUTDIR/Developer/Applications/Qt"
	EXE="$OUTDIR/Developer/Applications/Qt/${a}.app/Contents/MacOS/$a" #in the bundle
	#place it into the tools dir
	mkdir -p "$OUTDIR/Developer/Tools/Qt/"
	ln -s "/Developer/Applications/Qt/${a}.app" "$OUTDIR/Developer/Tools/Qt/${a}.app"
    elif [ -x "${BINDIR}/bin/${a}" ]; then
	EXE="$OUTDIR/usr/bin/$a-${VERSION_MAJOR}.${VERSION_MINOR}"
	mkdir -p `dirname $EXE`
	cp "${BINDIR}/bin/$a" "$EXE"
	ln -s "$a-${VERSION_MAJOR}.${VERSION_MINOR}" "$OUTDIR/usr/bin/$a"
	#place it into the tools dir
	mkdir -p "$OUTDIR/Developer/Tools/Qt/"
	ln -s "/usr/bin/$a-${VERSION_MAJOR}.${VERSION_MINOR}" "$OUTDIR/Developer/Tools/Qt/$a"
    fi
    [ -z "$EXE" ] && continue

    #configs
    if [ "$a" = "qmake" ]; then
	mkdir -p "$OUTDIR/usr/local/Qt${VERSION_MAJOR}.${VERSION_MINOR}/mkspecs"
	cp -PR $SRCDIR/mkspecs/* "$OUTDIR/usr/local/Qt${VERSION_MAJOR}.${VERSION_MINOR}/mkspecs"
	sed "s,qt_no_framework,,g" "$BINDIR/mkspecs/qconfig.pri" >"$OUTDIR/usr/local/Qt${VERSION_MAJOR}.${VERSION_MINOR}/mkspecs/qconfig.pri"
	cat >>"$OUTDIR/usr/local/Qt${VERSION_MAJOR}.${VERSION_MINOR}/mkspecs/qconfig.pri" <<EOF
CONFIG += no_mocdepend
EOF
	(cd "$OUTDIR/usr/local/Qt${VERSION_MAJOR}.${VERSION_MINOR}/mkspecs"; rm -f default; ln -s macx-xcode default)
	../libraries/fix_config_paths.pl -data "/usr/local/Qt${VERSION_MAJOR}.${VERSION_MINOR}" "$EXE" "/tmp/tmp.exe"
	cp "/tmp/tmp.exe" "$EXE"
	rm -f "/tmp/tmp.exe"
    elif [ "$a" = "assistant" ]; then
        mkdir -p "$OUTDIR/usr/lib"

        #copy assistant library
	[ -e "${BINDIR}/lib/libQtAssistantClient.a" ] && cp "${BINDIR}/lib/libQtAssistantClient.a" "$OUTDIR/usr/lib/libQtAssistantClient.a"

        #copy assistant headers
        mkdir -p "$OUTDIR/usr/include/QtAssistant"
        for header in `find "${BINDIR}/include/QtAssistant" -type f`; do
            if [ `basename "$header"` == "headers.pri" ]; then
                continue
            fi
            copyHeader "$header" "$OUTDIR/usr/include/QtAssistant"
        done
    elif [ "$a" = "Designer" ]; then
	mkdir -p "$OUTDIR/usr/lib/"
	[ -e "${BINDIR}/lib/libQtDesigner.${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.dylib" ] && ../libraries/fix_config_paths.pl "${BINDIR}/lib/libQtDesigner.${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.dylib" "$OUTDIR/usr/lib/libQtDesigner.${VERSION_MAJOR}.dylib"
	[ -e "${BINDIR}/lib/libQtDesignerComponents.${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.dylib" ] && ../libraries/fix_config_paths.pl "${BINDIR}/lib/libQtDesignerComponents.${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.dylib" "$OUTDIR/usr/lib/libQtDesignerComponents.${VERSION_MAJOR}.dylib"
	../libraries/fix_config_paths.pl "$EXE" "/tmp/tmp.exe"
	cp "/tmp/tmp.exe" "$EXE"
        rm -f /tmp/tmp.exe

        #copy designer headers
        copyHeaderDir "$BINDIR/include/QtDesigner" "$OUTDIR/usr/include/QtDesigner"
    else
	../libraries/fix_config_paths.pl "$EXE" "/tmp/tmp.exe"
	cp "/tmp/tmp.exe" "$EXE"
	rm -f /tmp/tmp.exe
    fi

    #perms
    strip "$EXE" >/dev/null 2>&1
    chmod a+x "$EXE" 
done

#copy q3porting.xml. qt3to4 looks for it in QLibraryInfo::DataPath and QLibraryInfo::PrefixPath
cp $BINDIR/tools/porting/src/q3porting.xml $OUTDIR/usr/local/Qt${VERSION_MAJOR}.${VERSION_MINOR}/q3porting.xml

#Handle the qtestlib
[ -e "${BINDIR}/lib/libQtTest.${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.dylib" ] && ../libraries/fix_config_paths.pl "${BINDIR}/lib/libQtTest.${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.dylib" "$OUTDIR/usr/lib/libQtTest.${VERSION_MAJOR}.dylib"

#copy QTest headers
copyHeaderDir "${BINDIR}/include/QtTest" "$OUTDIR/usr/include/QtTest"

# Finally handle QtUiTools
[ -e "${BINDIR}/lib/libQtUiTools.a" ] && cp "${BINDIR}/lib/libQtUiTools.a" "$OUTDIR/usr/lib/libQtUiTools.a"

# Copy it's headers as well
copyHeaderDir "${BINDIR}/include/QtUiTools" "$OUTDIR/usr/include/QtUiTools"

exit 0

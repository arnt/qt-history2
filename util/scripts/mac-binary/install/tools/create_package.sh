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

#copy tools
for a in qmake uic uic3 rcc moc assistant Linguist Designer rccdump lrelease qm2ts lupdate findtr; do
    EXE=
    if [ -d "${BINDIR}/bin/${a}.app" ]; then
	mkdir -p "$OUTDIR/Developer/Applications/Qt/"
	cp -r "${BINDIR}/bin/${a}.app" "$OUTDIR/Developer/Applications/Qt"
	EXE="$OUTDIR/Developer/Applications/Qt/${a}.app/Contents/MacOS/$a" #in the bundle
    elif [ -x "${BINDIR}/bin/${a}" ]; then
	EXE="$OUTDIR/usr/bin/$a-${VERSION_MAJOR}.${VERSION_MINOR}"
	mkdir -p `dirname $EXE`
	cp "${BINDIR}/bin/$a" "$EXE"
	ln -s "$a-${VERSION_MAJOR}.${VERSION_MINOR}" "$OUTDIR/usr/bin/$a"
	#place it into the applications dir (for now)
	mkdir -p "$OUTDIR/Developer/Applications/Qt/"
	ln -s "/bin/$a" "$OUTDIR/Developer/Applications/Qt/$a"
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
    elif [ "$a" = "Designer" ]; then
	mkdir -p "$OUTDIR/usr/lib/"
	[ -e "${BINDIR}/lib/libQtDesigner.1.dylib" ] && cp "${BINDIR}/lib/libQtDesigner.1.dylib" "$OUTDIR/usr/lib/"
	[ "$DO_DEBUG" = "no" ] && [ -e "${BINDIR}/lib/libQtDesigner_debug.1.dylib" ] && cp "${BINDIR}/lib/libQtDesigner_debug.1.dylib" "$OUTDIR/usr/lib/"
    fi

    #perms
    strip "$EXE" >/dev/null 2>&1
    chmod a+x "$EXE" 
done

exit 0
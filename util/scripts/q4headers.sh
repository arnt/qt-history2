#!/bin/sh

[ "$#" -gt 1 ] && QTDIR="$1" 
if [ -z "$QTDIR" ]; then
   echo "$0 <QTDIR>"
   exit 1
fi

for libinc in $QTDIR/include/*; do
    lib=`basename $libinc`
    [ "$lib" = "Qt" ] && continue
    for HEADER in `find $libinc -name '*.h'`; do
       echo "$HEADER" | grep _p >/dev/null 2>&1 && continue
       HEADER_IN="$HEADER" #first guess
       if [ `wc -l $HEADER_IN | awk '{print $1;}'` = "1" ]; then
   	  LINE=`head -1 $HEADER_IN`
	  if echo "$LINE" | grep "^#include" >/dev/null 2>&1; then
	      HEADER_IN=`dirname $HEADER_IN`/`echo $LINE | sed "s,^#include \"\([^\"]*\)\"\$,\1,"`
	  fi
       fi
       if [ -e "$HEADER_IN" ] && [ ! -z "$HEADER_IN" ]; then
	   FIXED=no
	   rm -f /tmp/file.tmp
	   exec 5<$HEADER_IN
	   while read -u5 C; do
	       if echo "$C" | grep '^ *# *include ' >/dev/null 2>&1; then
		    ORIG_FILE=
		    if echo $C | grep '<.*>' >/dev/null 2>&1; then
                         ORIG_FILE=`echo $C | sed 's,.*<\(.*\)>.*,\1,g'`
		    elif echo $C | grep '".*"' >/dev/null 2>&1; then
                         ORIG_FILE=`echo $C | sed 's,.*"\(.*\)".*,\1,g'`
                    fi
		    if [ -e "$libinc/$ORIG_FILE" ] && false; then
			echo "$C" >>/tmp/file.tmp
		    else
			real_lib=
			for otherinc in $QTDIR/include/*; do
			    other_lib=`basename $otherinc`
			    [ "$other_lib" = "Qt" ] && continue
			    if [ -e "$otherinc/$ORIG_FILE" ]; then
				real_lib="$other_lib"
				break
			    fi
			done
	                if [ ! -z "$real_lib" ]; then
			    C=`echo $C | sed "s,$ORIG_FILE,$real_lib/$ORIG_FILE,g"`
			    FIXED=yes
                        fi
               	        echo "$C" >>/tmp/file.tmp
		    fi
	       else
                    echo "$C" >>/tmp/file.tmp
	       fi
	   done
	   if [ "$FIXED" = "yes" ]; then
	       P4_HEADER_IN=`echo $HEADER_IN | sed "s,^.*/\.\./\.\./,,"`
	       (cd $QTDIR && p4 edit "$P4_HEADER_IN") && mv /tmp/file.tmp "$HEADER_IN"
           fi
       fi
    done
done

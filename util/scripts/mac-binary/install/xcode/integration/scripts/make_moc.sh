#!/bin/sh

CFG_PREPROCESS_FLAGS=
CFG_MOC="/usr/bin/moc"
CFG_INPUT=
CFG_OUTPUT=

while [ -n "$1" ]; do
   CACHE_1="$1"
   OPTION=
   ARG=
   case $1 in
   -*=*) #gnu style
     OPTION=`echo $1 | sed 's,-\([^=]*\)=\(.*\),\1,'`
     ARG=`echo $1 | sed 's,-\([^=]*\)=\(.*\),\2,'`
     ;;
   -output|-input|-moc) #second arg style
     OPTION=`echo $1 | sed 's,-\([^=]*\),\1,'`
     shift
     ARG=$1
     ;;          
   esac
   shift
   case "$OPTION" in
   moc) CFG_MOC="$ARG" ;;
   output) CFG_OUTPUT="$ARG" ;;
   input)  CFG_INPUT="$ARG" ;;
   help|*)
       [ "$OPTION" = "help" ] || echo "Unknown option $CACHE_1!" 
       echo "Help!!"
       exit 888;
       ;;
   esac  
done

if [ -z "$CFG_INPUT" ] || [ -z "$CFG_OUTPUT" ]; then
   echo "No input/output file specified."
   exit 1
fi
[ "$CFG_OUTPUT" -nt "$CFG_INPUT" ] && exit 0

cd "${SOURCE_ROOT}"
for a in $OTHER_CPLUSPLUSFLAGS; do
   case "$a" in
     -D*|-I*)
       CFG_PREPROCESS_FLAGS="$CFG_PREPROCESS_FLAGS ${a}" ;;
     *) ;;
   esac
done
for a in $GCC_PREPROCESSOR_DEFINITIONS; do
   CFG_PREPROCESS_FLAGS="$CFG_PREPROCESS_FLAGS -D${a}"
done
CFG_PREPROCESS_FLAGS="$CFG_PREPROCESS_FLAGS -I${SOURCE_ROOT}" 

#do the moc
echo >"$CFG_OUTPUT"
if [ -e "$CFG_INPUT" ]; then
   LINE=0
   LINES=`wc -l "$CFG_INPUT" | awk '{ print $1; }'`
   while [ "$LINE" -lt "$LINES" ]; do
      LINE=$((LINE+1))
      SOURCE=`head -$LINE ${CFG_INPUT} | tail -1`

      FILE=`echo $SOURCE | sed "s,^#include \"\([^\"]*\)\"$,\1,"`
      if [ -f "$FILE" ]; then
          $CFG_MOC $CFG_PREPROCESS_FLAGS "$FILE" >>"$CFG_OUTPUT"
      fi
   done
fi

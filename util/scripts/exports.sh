#!/bin/sh

#ARGUMENTS
DIR=$1
if [ -z "$DIR" ]; then
    DIR=$PWD
elif [ ! -d "$DIR" ]; then
   echo "$DIR doesn't exist.."
   exit 666
fi
DIR=`(cd $DIR; /bin/pwd)` #absolute now..
EXPORT=$2
[ -z "$EXPORT" ] && EXPORT="Q_EXPORT"

#PATHS
OUTPWD=$PWD
cd $DIR

#AWK SCRIPT
if [ ! -e "$OUTPWD/parse-private.awk" ]; then
   true    
fi

#FIND
echo -n "Finding code.."
rm -f $OUTPWD/exports
CXXFLAGS=
#if [ -f "$DIR/Makefile" ]; then
#  for var in CFLAGS INCPATH; do
#     CXXFLAGS="$CXXFLAGS $(grep "^$var *=" $DIR/Makefile | cut -d'=' -f2-)"
#  done
#fi
for export in $EXPORT; do
   EXTRA_CXXFLAGS="$CXXFLAGS -D${EXPORT}=${EXPORT}"
done
for suff in cpp c h; do
  FILES=`find . -name "*.${suff}"`
  rm -f $OUTPWD/tmp.out $OUTPWD/tmp.local
  for file in $FILES; do
    if [ -e "$OUTPWD/parse-private.awk" ] && which awk >/dev/null 2>&1; then
        awk -f "$OUTPWD/parse-private.awk" "$file" >>$OUTPWD/tmp.out
    fi
    for export in $EXPORT; do
        echo -n .
        gcc -E $CXXFLAGS $EXTRA_CXXFLAGS $file 2>/dev/null | grep "$export " | sed "s,$export ,,g" | grep -v '^#' >>$OUTPWD/tmp.out
    done
    size=`wc -c $OUTPWD/tmp.out | awk '{print $1;}'`
    [ "$size" = "0" ] && continue;
  done
  sort $OUTPWD/tmp.out | uniq >>$OUTPWD/exports
  rm -f $OUTPWD/tmp.out
  echo -n .
done    
echo  

#PROCESS
lines=`wc -l $OUTPWD/exports | awk '{print $1;}'`
echo -n "Processing code ($lines).."
line=0
rm -f $OUTPWD/class.exp $OUTPWD/other.exp
while [ "$line" -le "$lines" ]; do
   LINE=`head -${line} $OUTPWD/exports | tail -1 | sed "s,//.*$,,g"`
   if echo $LINE | grep 'template class .*<.*>' >/dev/null 2>&1; then
      TEMPL=`echo $LINE | sed 's,.*template class \(.*<.*>\).*,\1,g'`
      TEMPL=`echo $TEMPL | sed 's/[<>, ]/?/g'`
      echo "$TEMPL" >>$OUTPWD/class.exp
   elif echo $LINE | grep 'template *<.*> \(struct\|class\) \([a-zA-Z0-9_]*\).*' >/dev/null 2>&1; then
      CLASS=`echo $LINE | sed 's, *template \?\(<.*>\) \(struct\|class\) *\([a-zA-Z0-9_]*\)[ :{]\?.*,\3\1,'`
      CLASS=`echo $CLASS | sed 's/[<>, ]/?/g'`
      echo "$CLASS" >>$OUTPWD/class.exp
   elif echo $LINE | grep '^ *\(struct\|class\) *\([a-zA-Z0-9_]*\).*' >/dev/null 2>&1; then
      CLASS=`echo $LINE | sed 's, \?\(struct\|class\) *\([a-zA-Z0-9_]*\)[ :{]\?.*,\2,'`
      echo "$CLASS" >>$OUTPWD/class.exp
   elif echo $LINE | grep '.*[ *&]*[a-zA-Z0-9_]* *(' >/dev/null 2>&1; then
      FUNC=`echo $LINE |  sed 's,.*[ &*]\(..*\) *(.*,\1,g' | sed 's, ,,g'`
      echo $FUNC | grep '^operator' >/dev/null 2>&1 || echo "$FUNC" >>$OUTPWD/other.exp
   elif echo $LINE | grep ";$" >/dev/null 2>&1; then
       INST="$LINE"
       echo "$INST" | grep '=' >/dev/null 2>&1 && INST="$(echo $INST | sed "s, *=.*$,,");"
       INST=`echo $INST | sed "s,.* [*&]\?\(.*\);.*,\1,g"`
       echo "$INST" >>$OUTPWD/other.exp
   elif [ ! -z "$LINE" ] && [ "$LINE" != "Q_EXPORT" ]; then
       true
#       echo "Cannot handle '$LINE'"
   fi
   echo -n .
   line=$((line+1))
done
echo

#CREATE
echo -n "Creating libqt.map.."
rm -f $OUTPWD/libqt.map
echo "{" >>$OUTPWD/libqt.map
echo "  global:" >>$OUTPWD/libqt.map
echo "  extern \"C++\"" >>$OUTPWD/libqt.map
echo "  {" >>$OUTPWD/libqt.map
prev=0
for file in $OUTPWD/other.exp $OUTPWD/class.exp $DIR/global.exp; do
  if [ -e "$file" ]; then
    sort $file | uniq >$OUTPWD/tmp.out
    lines=`wc -l $OUTPWD/tmp.out | awk '{print $1;}'`
    line=0
    while [ "$line" -le "$lines" ]; do
        match=`head -${line} $OUTPWD/tmp.out | tail -1 | sed "s,//.*$,,g"`
        if [ ! -z "$match" ]; then
	   if [ "$file" = "$OUTPWD/class.exp" ]; then
	      OUTPUT="$match::* $match?virtual?table $match?type_info?node $match?type_info?function vtable?for?$match typeinfo?for?$match non-virtual?thunk?to?$match::*"
	   else
	      OUTPUT="$match*"
	   fi
	   for out in $OUTPUT; do
	       if [ "$prev" = "1" ]; then
		  echo -e ";" >>$OUTPWD/libqt.map
	       else
	          prev="1"
	       fi
	       echo -n "    $out" >>$OUTPWD/libqt.map
           done
        fi
        line=$((line+1))
        echo -n .
    done
    #let's just export all operator functions.. bit of a hack
    if [ "$file" = "$OUTPWD/other.exp" ]; then
      if [ "$prev" = "1" ]; then
	echo -e ";" >>$OUTPWD/libqt.map
      else
	prev="1"
      fi
      echo -n "    operator*" >>$OUTPWD/libqt.map
    fi
  fi
done
echo -e "\n  };" >>$OUTPWD/libqt.map
echo "  local:" >>$OUTPWD/libqt.map
echo "  extern \"C++\"" >>$OUTPWD/libqt.map
echo "  {" >>$OUTPWD/libqt.map
echo "    *" >>$OUTPWD/libqt.map
if [ -e "$DIR/local.exp" ]; then
    sort $DIR/local.exp | uniq >$OUTPWD/tmp.out
    lines=`wc -l $OUTPWD/tmp.out | awk '{print $1;}'`
    line=0
    prev=0
    while [ "$line" -le "$lines" ]; do
        match=`head -${line} $OUTPWD/tmp.out | tail -1 | sed "s,//.*$,,g"`
	if [ "$prev" = "1" ]; then
	  echo -e ";" >>$OUTPWD/libqt.map
	else
	  prev="1"
	fi
	echo -n "    $out" >>$OUTPWD/libqt.map
        line=$((line+1))
        echo -n .
    done
fi
echo "  };" >>$OUTPWD/libqt.map
echo "};" >>$OUTPWD/libqt.map
echo

#CLEANUP
rm -f $OUTPWD/class.exp $OUTPWD/other.exp
rm -f $OUTPWD/tmp.out
rm -f $OUTPWD/exports
rm -f $OUTPWD/parse-private.awk

echo "Done!"

  

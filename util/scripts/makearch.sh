#!/bin/sh
# 
# Copyright 1996 Troll Tech AS.  All rights reserved.
#

# Note that this script is alway invoked in qt/src
# since this is shell not perl, we can't really check it

unset MAKELEVEL
unset MAKEFLAGS
unset MFLAGS

set -e

[ -d tools -a -d kernel -a -d widgets -a -d dialogs -a -d moc ] || exit 1;

# okay, a little setup
cd ..
BASE=`pwd`

# prepare the directories for content
if [ -d include ]; then
    rm -rf include/*.h
else
    mkdir include
fi

if [ -d arch ]; then
    rm -rf arch/*
else
    mkdir arch
fi
mkdir arch/template
mkdir arch/template/library
mkdir arch/template/examples
mkdir arch/template/tutorial
mkdir arch/template/moc

cp ${BASE}/util/scripts/propagate arch/template/propagate

# fill them
for a in tools kernel widgets dialogs ; do
    cd ${BASE}/src/${a}
    beta=`make -s showbeta`
    if [ "$beta" = "" ]; then
	beta="s/XYZ/XYZ/g";
    else
	beta=`echo $beta | sed -e 's/ /\\\|/g' -e 's/\./\\\./g'`
	beta="s/$beta//g"
    fi
    ln -s `make -s showheaders | sed $beta | fmt -1 | sed 's-^-../src/'${a}'/-'` ${BASE}/include
    ln -s `make -s showheaders showsources | sed $beta | fmt -1 | sed 's-^-../../../src/'${a}'/-'` ${BASE}/arch/template/library
done

cd ${BASE}/examples
for a in `make -s showdirs` ; do
    cd ${BASE}/examples/${a}
    mkdir ${BASE}/arch/template/examples/${a}
    ln -s `echo * | fmt -1 | grep -v Makefile | \
	sed 's-^-../../../../examples/'${a}'/-'` \
	${BASE}/arch/template/examples/${a}
    ln Makefile ${BASE}/arch/template/examples/${a}/Makefile
done
ln ${BASE}/examples/Makefile ${BASE}/arch/template/examples/Makefile
cd ${BASE}/tutorial
for a in `make -s showdirs` ; do
    cd ${BASE}/tutorial/${a}
    mkdir ${BASE}/arch/template/tutorial/${a}
    ln -s `make -s showfiles | fmt -1 | grep -v Makefile | \
	sed 's-^-../../../../tutorial/'${a}'/-'` \
	${BASE}/arch/template/tutorial/${a}
    ln Makefile ${BASE}/arch/template/tutorial/${a}/Makefile
done
ln ${BASE}/tutorial/Makefile ${BASE}/arch/template/tutorial/Makefile
cd ${BASE}/arch/template/moc
ln -s ../../../src/moc/moc.[l1y] ../../../src/moc/moc.cpp ../../../src/moc/lex.yy.c .
sed -e 's-\.\./tools/-../library/-' < ../../../src/moc/Makefile > Makefile

# woo hoo
# make the template makefile
cd ${BASE}/arch/template

exec > Makefile

cat <<EOF
# -*- makefile -*-
# This Makefile is a template for filling in to port Qt to a new platform.
# .../arch/*/Makefile are all generated from this one (including this one,
# in a way).  Read PORTING for instructions.
#

# CC - must be a modern c++ compiler.  cfront is almost certainly out.
CC = gcc

# CFLAGS - the basic flags for compiling.
CFLAGS = -O2 -fno-strength-reduce -W -Wtemplate-debugging -Wparentheses \\
	-Wuninitialized -Wchar-subscripts -Wformat -Wtrigraphs \\
	-Wcomment -Wswitch -Wunused -Wreturn-type -Wimplicit \\
	-Wpointer-arith -Wsynth -Wconversion -Wno-overloaded-virtual

# LFLAGS - the flags the linker needs
# NOTE: -L.../lib is prepended by ./propagate so everyone can find libqt
LFLAGS = -lqt

all: moc library tutorial examples

moc library tutorial examples: variables

library tutorial examples: FORCE
	cd \$@; \$(MAKE)

moc: FORCE
	cd \$@; \$(MAKE)
	cp moc/moc ../../bin/moc

clean:
	rm variables
	cd moc; \$(MAKE) clean
	cd library; \$(MAKE) clean
	cd tutorial; \$(MAKE) clean
	cd tutorial; \$(MAKE) clean

depend:
	cd moc; \$(MAKE) depend
	cd library; \$(MAKE) depend
	cd tutorial; \$(MAKE) depend
	cd tutorial; \$(MAKE) depend

variables: Makefile
	CFLAGS="\$(CFLAGS)" CC="\$(CC)" LFLAGS="\$(LFLAGS)" ./propagate
	touch variables

dep: depend

FORCE:


EOF

cd ${BASE}/arch/template/library
exec > Makefile

cat <<EOF
# \$Source\$

# The Qt meta object support files
#
# see moc(1) or http://www.troll.no/qt/metaobjects.html for info about
# meta objects and the meta object ocmpier
EOF

# brute force rules

METAHEADERS=`(grep -l Q_OBJECT *.h ; echo qobjdefs.h qsignal.h ) | \
		fmt -1 | sort | uniq -u`

echo $METAHEADERS | fmt -1 | sed -e 's/^q/m/' -e 's/\.h$/.cpp/' | fmt -66 | \
    sed -e 's/^/	  /' \
	-e '1 s/	  /METASRC = /' \
	-e 's/$/ \\/' \
	-e '$ s/ \\//'

echo $METAHEADERS | fmt -1 | sed -e 's/^q/m/' -e 's/\.h$/.o/' | fmt -66 | \
    sed -e 's/^/	  /' \
	-e '1 s/	  /METAOBJ = /' \
	-e 's/$/ \\/' \
	-e '$ s/ \\//'

echo
echo '# The C++ header files'
echo *.h | fmt -66 | sed \
	-e 's/^/	  /' \
	-e '1 s/	  /HEADERS = /' \
	-e 's/$/ \\/' \
	-e '$ s/ \\//'

echo
echo '# The C++ source files that are edited by humans'
echo *.cpp | fmt -66 | sed \
	-e 's/^/	  /' \
	-e '1 s/	  /SOURCES = /' \
	-e 's/$/ \\/' \
	-e '$ s/ \\//'

echo
echo '# The object code files from SOURCES above'
echo *.cpp | fmt -1 | sed 's/\.cpp$/.o/' | fmt -66 | sed \
	-e 's/^/	  /' \
	-e '1 s/	  /OBJECTS = /' \
	-e 's/$/ \\/' \
	-e '$ s/ \\//' \

cat  <<EOF

# You probably need to change the next few lines.  In particular,
# we add an extra gcc option in the rule - some hack!

.SUFFIXES:
.SUFFIXES: .cpp \$(SUFFIXES)

.cpp.o:
	\$(CC) -I\$(INCDIR) -c \$(CFLAGS) -fpic \$<

# VERSION - the version number of the shared library, where applicable
VERSION=0.9.6

# The main rule - please do not hardcode the version number here,
# and please do not change it.  Until 1.0, Qt may not be forwards
# or backwards compatible anyway, so we have chosen to disregard
# the "proper" rules of library numbering.

library: \$(OBJECTS) \$(METAOBJ)
	-rm -f ../../lib/libqt.so.\${VERSION}
	\$(CC) -shared -Wl,-soname,libqt.so.0 \\
		-o ../../../lib/libqt.so.\${VERSION} \\
		\$(OBJECTS) \$(METAOBJ) -lX11 -lgcc -lc

depend:
	makedepend \$(SOURCES) 2> /dev/null

# You should not need to change anything beyond this point

dep: depend

clean:
	-/bin/rm -f *.o *.bak *BAK *~ *% \$(SRCMETA)

# For maximum compatibility with old versions of make, we generate
# a separate rule for each meta object source file.

# Though you can see what is in these files, we urge you not to use this
# knowledge in your programs.

# METAHEADERS is the list of header files included by all the meta object
# source files

EOF

gcc -E qmetaobj.h | \
    awk '/^# [0-9]+ [^/]*$/{print $3}' | \
    tr -d '"' | \
    sort | uniq | fgrep -v qmetaobj.h | fmt -66 | \
    sed -e 's/^/	/' \
	-e '1 s/	/METAHEADERS = /' \
	-e 's/$/ \\/' \
	-e '$ s/ \\//'
echo 

for a in $METAHEADERS ; do
    METASRC=`echo $a | sed -e 's/^q/m/' -e 's/h$/cpp/'`
    echo `echo ${METASRC} | sed 's/cpp$/o/'`: ${METASRC} 
    echo ${METASRC}: ${a} '$(METAHEADERS)'
    echo '	$(MOC)' -o ${METASRC} ${a}
    echo 
done

cat  <<EOF

# DO NOT DELETE THIS LINE -- make depend depends on it.

# What is here now is accurate as far as it goes, but does not include
# system header files.

EOF

for a in *.cpp ; do
    gcc -E $a | awk '/^# [0-9]+ [^/]*$/{print $3}' | tr -d '"' | sort | \
    uniq | sed -e '1 s/^/'${a}': /' -e '1 s/\.cpp:/.o:/' | fmt -66 | sed \
	-e '2,$ s/^/	/' \
	-e 's/$/ \\/' \
	-e '$ s/ \\//'
done

exec 1>&2

cd ${BASE}/arch

exec > Makefile

cat << EOF
# this makefile will make architecture link trees
#
# e.g. say 'make linux-shared' to make a link tree suitable for making
# a shared library for linux or 'make solaris-static' to make a static
# library for solaris 2.

all:
	echo say e.g. make linux-shared to make an architecture tree

# pseudo target to force mkdir for the others

FORCE:


EOF

for a in ../makefiles/*[a-z] ; do
    PLATFORM=`basename $a`
    cat << EOF
${PLATFORM}: FORCE
	mkdir ${PLATFORM}
	cd template ; tar cf - . | ( cd ../${PLATFORM} ; tar xf - )
EOF
    [ -s $a ] && echo "	( cd ${PLATFORM} ; patch -p1 -s < ../../makefiles/${PLATFORM} )"
    echo

done

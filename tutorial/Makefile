####### This section was automatically generated from
#######    /home/hanord/qt/tutorial/Makefile for building on
#######     on Tue Apr  1 19:27:47 MET DST 1997

INCDIR = $(QTDIR)/include
CFLAGS =  -O2 -fno-strength-reduce -Wall -W
LIBCFLAGS = -fPIC
YACCCFLAGS = -Wno-unused -Wno-parentheses
LFLAGS = -L$(QTDIR)/lib -lqt
CC = gcc
MOC = moc
SHELL =	/bin/sh

####### End of automatically generated section
#
# $Source: /tmp/cvs/qt/tutorial/Makefile,v $
#

####### Directories

SUBDIRS =	t1 \
		t2 \
		t3 \
		t4 \
		t5 \
		t6 \
		t7 \
		t8 \
		t9 \
		t10 \
		t11 \
		t12 \
		t13 \
		t14

####### Targets

all: $(SUBDIRS)

$(SUBDIRS): FORCE
	cd $@; $(MAKE)

showdirs:
	@echo $(SUBDIRS)

galore:
	for a in */* ; do [ -f $$a -a -x $$a ] && ( cd `dirname $$a` ; ../$$a & ) ; done

tmake:
	for i in $(SUBDIRS); do ( cd $$i ; tmake $$i.pro -o $$i.mak ) ; done

clean:
	for i in $(SUBDIRS); do ( cd $$i ; $(MAKE) clean ) ; done

FORCE:

#############################################################################
# $Id: //depot/qt/main/GNUmakefile.inc#22 $
#
# Unix Makefile, requires GNU make (gmake).
#
# Copyright (C) 1998 by Troll Tech AS.  All rights reserved.
#
#############################################################################


ifdef QT_EMBEDDED
ifeq ($(PROJECT),qt)
PROJECT=qt-e
endif
ifeq ($(QT_EMBEDDED),mips)
# MIPS_OPT="-mcpu=r4000 -mips3" # Doesn't work?
TMAKEHACKEXTRA="DASHMIPS=-mips" "TMAKE_CXXFLAGS+=$(MIPS_OPT) -fnonnull-objects -fshort-enums -fshared-data" "TMAKE_LINK_SHLIB=mipsel-linux-g++ -fnonnull-objects" "TMAKE_LINK=mipsel-linux-g++" "TMAKE_CC=mipsel-linux-gcc $(MIPS_OPT)" "TMAKE_CXX=mipsel-linux-g++"
endif
ifeq ($(LIBTARGET),1)
TMAKEHACK="DESTDIR=../lib-e-$(QT_EMBEDDED)" $(TMAKEHACKEXTRA)
else
TMAKEHACK="TMAKE_LIBDIR_QT=$(QTDIR)/lib-e-$(QT_EMBEDDED)" $(TMAKEHACKEXTRA)
endif
PROJOBJ=e-$(QT_EMBEDDED)/
PROJMAK=-e-$(QT_EMBEDDED)
TMAKEPATH=$(TMAKEDIR)/lib/linux-qws-g++
export TMAKEPATH
ifndef QT_TARGET
QT_TARGET=release
endif
else
# Should do this too...
#PROJOBJ=x11/
#PROJMAK=-x11
endif


ifndef QT_TARGET
QT_TARGET=debug
endif

default: $(QT_TARGET)

ifndef LIBTARGET
LIBTARGET = 0
endif
ifeq ($(LIBTARGET),0)
VCTEMPLATE = vcapp
else
VCTEMPLATE = vclib
endif


SYNCQT = $(QTDIR)/bin/syncqt

project: $(PROJECT).pro

$(PROJECT).pro:
	progen -o "$(PROJECT).pro" -n "$(PROJECT)"


%: GNUmakefile$(PROJMAK).% $(MORETARGETS)
	$(MAKE) -j4 -f GNUmakefile$(PROJMAK).$*

static: static-debug

gcc: gcc-debug

GNUmakefile$(PROJMAK).debug: $(PROJECT).pro
ifeq ($(LIBTARGET),1)
	$(SYNCQT)
endif
	@mkdir -p .obj/$(PROJOBJ)debug
	tmake $(PROJECT).pro -o GNUmakefile$(PROJMAK).debug \
	    "CONFIG+=debug" "OBJECTS_DIR=.obj/$(PROJOBJ)debug" $(TMAKEHACK)

GNUmakefile$(PROJMAK).release: $(PROJECT).pro
ifeq ($(LIBTARGET),1)
	$(SYNCQT)
endif
	@mkdir -p .obj/$(PROJOBJ)release
	tmake $(PROJECT).pro -o GNUmakefile$(PROJMAK).release \
	    "CONFIG+=release" "OBJECTS_DIR=.obj/$(PROJOBJ)release" $(TMAKEHACK)

GNUmakefile$(PROJMAK).release-debug: $(PROJECT).pro GNUmakefile
ifeq ($(LIBTARGET),1)
	$(SYNCQT)
endif
	@mkdir -p .obj/$(PROJOBJ)release-debug
	tmake $(PROJECT).pro -o GNUmakefile$(PROJMAK).release-debug \
	    "CONFIG+=release" "OBJECTS_DIR=.obj/$(PROJOBJ)release-debug" $(TMAKEHACK)
	perl -pi \
	    -e 's/^([CL]X*)FLAGS\s*=/$$1FLAGS=-g /' \
		GNUmakefile$(PROJMAK).release-debug

GNUmakefile$(PROJMAK).static-debug: $(PROJECT).pro GNUmakefile
ifeq ($(LIBTARGET),1)
	$(SYNCQT)
endif
	@mkdir -p .obj/$(PROJOBJ)static-debug
	tmake $(PROJECT).pro -o GNUmakefile$(PROJMAK).static-debug \
	    "CONFIG+=staticlib debug" "OBJECTS_DIR=.obj/$(PROJOBJ)static-debug"

GNUmakefile$(PROJMAK).static-release: $(PROJECT).pro GNUmakefile
ifeq ($(LIBTARGET),1)
	$(SYNCQT)
endif
	@mkdir -p .obj/$(PROJOBJ)static-release
	tmake $(PROJECT).pro -o GNUmakefile$(PROJMAK).static-release \
	    "CONFIG+=staticlib release" "OBJECTS_DIR=.obj/$(PROJOBJ)static-release" $(TMAKEHACK)

GNUmakefile$(PROJMAK).profile: $(PROJECT).pro GNUmakefile
ifeq ($(LIBTARGET),1)
	$(SYNCQT)
endif
	@mkdir -p .obj/$(PROJOBJ)profile
	tmake $(PROJECT).pro -o GNUmakefile$(PROJMAK).profile \
	    "CONFIG+=staticlib release" "OBJECTS_DIR=.obj/$(PROJOBJ)profile" $(TMAKEHACK)
	perl -pi \
	    -e 's/^([CL]X*)FLAGS\s*=/$$1FLAGS=-g -pg /; s|-lqt|\$$(QTDIR)/lib$(PROJMAK)/libqt.a \$$(QTDIR)/src/.obj/$(PROJOBJ)profile/libfreetype.a|' \
		GNUmakefile$(PROJMAK).profile

GNUmakefile$(PROJMAK).gcc-debug: $(PROJECT).pro GNUmakefile
ifeq ($(LIBTARGET),1)
	$(SYNCQT)
endif
	@mkdir -p .obj/$(PROJOBJ)gcc-debug
	tmake $(PROJECT).pro -o GNUmakefile$(PROJMAK).gcc-debug \
	    "CONFIG+=debug" "TMAKE_CXX=gcc" "OBJECTS_DIR=.obj/$(PROJOBJ)gcc-debug" $(TMAKEHACK)

GNUmakefile$(PROJMAK).gcc-release: $(PROJECT).pro GNUmakefile
ifeq ($(LIBTARGET),1)
	$(SYNCQT)
endif
	@mkdir -p .obj/$(PROJOBJ)gcc-release
	tmake $(PROJECT).pro -o GNUmakefile$(PROJMAK).gcc-release \
	    "CONFIG+=release" "TMAKE_CXX=gcc" "OBJECTS_DIR=.obj/$(PROJOBJ)gcc-release"

GNUmakefile$(PROJMAK).gcc-static-debug: $(PROJECT).pro GNUmakefile
ifeq ($(LIBTARGET),1)
	$(SYNCQT)
endif
	@mkdir -p .obj/$(PROJOBJ)gcc-static-debug
	tmake $(PROJECT).pro -o GNUmakefile$(PROJMAK).gcc-static-debug \
	    "CONFIG+=staticlib debug" "TMAKE_CXX=gcc" "OBJECTS_DIR=.obj/$(PROJOBJ)gcc-static-debug" $(TMAKEHACK)

GNUmakefile$(PROJMAK).gcc-static-release: $(PROJECT).pro GNUmakefile
ifeq ($(LIBTARGET),1)
	$(SYNCQT)
endif
	@mkdir -p .obj/$(PROJOBJ)gcc-static-release
	tmake $(PROJECT).pro -o GNUmakefile$(PROJMAK).gcc-static-release \
	    "CONFIG+=staticlib release" "TMAKE_CXX=gcc" "OBJECTS_DIR=.obj/$(PROJOBJ)gcc-static-release" $(TMAKEHACK)



showfiles:
	@tmake $(PROJECT).pro -e 'Expand("HEADERS","SOURCES")'

showheaders:
	@tmake $(PROJECT).pro -e 'Expand("HEADERS")'

showsources:
	@tmake $(PROJECT).pro -e 'Expand("SOURCES")'

tags: $(PROJECT).pro
	@tmake $(PROJECT).pro -e 'Expand("HEADERS","SOURCES")' \
		| fmt -1 | etags --c++ -


include:
	$(SYNCQT)

update:
	cvs update -d
	$(MAKE) depend

updateqt:
	$(MAKE) -C $(QTDIR)/src update

depend:
	-rm -f GNUmakefile*.*debug GNUmakefile*.*release GNUmakefile*.profile $(MOREDEP)

mocclean:
	-rm -f */q*.moc */moc_q*.cpp *.moc moc_*.cpp

clean: GNUmakefile$(PROJMAK).debug
	$(MAKE) -f GNUmakefile$(PROJMAK).debug clean
	-rm -f GNUmakefile*.*debug GNUmakefile*.*release GNUmakefile*.profile $(MOREDEP) $(MORECLEAN)
	-rm -f core *~
	-rm -r -f .obj

help:
	@cat $(QTDIR)/make.help

win windows:
	nmake $(MAKEFLAGS)

bwin:
	bmake $(MAKEFLAGS)

FORCE:

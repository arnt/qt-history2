#############################################################################
# $Id: //depot/qt/main/GNUmakefile.inc#35 $
#
# Unix Makefile, requires GNU make (gmake).
#
# Copyright (C) 1998 by Trolltech AS.  All rights reserved.
#
#############################################################################

ifdef INSURE
TMAKEINSURE="TMAKE_LINK_SHLIB=insure" "TMAKE_LINK=insure" "TMAKE_CC=insure" "TMAKE_CXX=insure"
endif

ifdef QT_EMBEDDED

PROFLIBS=\$$(QTDIR)/src/.obj/$(PROJOBJ)profile/libfreetype.a \$$(QTDIR)/src/.obj/$(PROJOBJ)profile/libpng.a \$$(QTDIR)/src/.obj/$(PROJOBJ)profile/libz.a \$$(QTDIR)/src/3rdparty/libmng/libmng.a \$$(QTDIR)/src/3rdparty/jpgsrc/libjpeg.a
ifeq ($(PROJECT),qt)
PROJECT=qt
endif
ifeq ($(QT_EMBEDDED),mips)
# MIPS_OPT="-mcpu=r4000 -mips3" # Doesn't work?
TMAKEHACKEXTRA="DASHCROSS=-mips" "TMAKE_CXXFLAGS+=$(MIPS_OPT) -fnonnull-objects -fshort-enums -fshared-data" "TMAKE_LINK_SHLIB=mipsel-linux-g++ -fnonnull-objects" "TMAKE_LINK=mipsel-linux-g++" "TMAKE_CC=mipsel-linux-gcc $(MIPS_OPT)" "TMAKE_CXX=mipsel-linux-g++"
ifndef QT_TARGET
QT_TARGET=release
endif
endif
ifeq ($(QT_EMBEDDED),arm)
TMAKEHACKEXTRA="DASHCROSS=-arm" "TMAKE_CXXFLAGS+=$(MIPS_OPT) -fshort-enums -fshared-data" "TMAKE_LINK_SHLIB=arm-linux-g++ " "TMAKE_LINK=arm-linux-g++" "TMAKE_CC=arm-linux-gcc $(MIPS_OPT)" "TMAKE_CXX=arm-linux-g++"
ifndef QT_TARGET
QT_TARGET=debug
endif
endif
ifeq ($(LIBTARGET),1)
TMAKEHACK=$(TMAKEINSURE) "DESTDIR=$(QTDIR)/lib-e-$(QT_EMBEDDED)" $(TMAKEHACKEXTRA)
else
TMAKEHACK=$(TMAKEINSURE) "TMAKE_LIBDIR_QT=$(QTDIR)/lib-e-$(QT_EMBEDDED)" $(TMAKEHACKEXTRA)
endif
PROJOBJ=e-$(QT_EMBEDDED)/
PROJMAK=-e-$(QT_EMBEDDED)
TMAKEPATH=$(TMAKEDIR)/lib/qws/linux-$(QT_EMBEDDED)-g++
export TMAKEPATH

TMAKEPREHACK="CONFIG+=embedded" "CONFIG+=internal"

else

TMAKEHACK=$(TMAKEINSURE)
TMAKEPREHACK="CONFIG+=internal"
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
	$(MAKE) -j4 PROJMAK=$(PROJMAK) -f GNUmakefile$(PROJMAK).$*

static: static-debug

gcc: gcc-debug

GNUmakefile$(PROJMAK).debug: $(PROJECT).pro
ifeq ($(LIBTARGET),1)
	$(SYNCQT)
endif
	@mkdir -p .obj/$(PROJOBJ)debug
	tmake $(TMAKEPREHACK) "OBJECTS_DIR=.obj/$(PROJOBJ)debug" $(PROJECT).pro -o GNUmakefile$(PROJMAK).debug \
	    "CONFIG+=debug" $(TMAKEHACK)

GNUmakefile$(PROJMAK).release: $(PROJECT).pro
ifeq ($(LIBTARGET),1)
	$(SYNCQT)
endif
	@mkdir -p .obj/$(PROJOBJ)release
	tmake $(TMAKEPREHACK) "OBJECTS_DIR=.obj/$(PROJOBJ)release" $(PROJECT).pro -o GNUmakefile$(PROJMAK).release \
	    "CONFIG+=release" $(TMAKEHACK)

GNUmakefile$(PROJMAK).release-debug: $(PROJECT).pro GNUmakefile
ifeq ($(LIBTARGET),1)
	$(SYNCQT)
endif
	@mkdir -p .obj/$(PROJOBJ)release-debug
	tmake $(TMAKEPREHACK) "OBJECTS_DIR=.obj/$(PROJOBJ)release-debug" $(PROJECT).pro -o GNUmakefile$(PROJMAK).release-debug \
	    "CONFIG+=release" $(TMAKEHACK)
	perl -pi \
	    -e 's/^([CL]X*)FLAGS\s*=/$$1FLAGS=-g /' \
		GNUmakefile$(PROJMAK).release-debug

GNUmakefile$(PROJMAK).static-debug: $(PROJECT).pro GNUmakefile
ifeq ($(LIBTARGET),1)
	$(SYNCQT)
endif
	@mkdir -p .obj/$(PROJOBJ)static-debug
	tmake $(TMAKEPREHACK) "OBJECTS_DIR=.obj/$(PROJOBJ)static-debug" $(PROJECT).pro -o GNUmakefile$(PROJMAK).static-debug \
	    "CONFIG+=staticlib debug" $(TMAKEHACK)

GNUmakefile$(PROJMAK).static-release: $(PROJECT).pro GNUmakefile
ifeq ($(LIBTARGET),1)
	$(SYNCQT)
endif
	@mkdir -p .obj/$(PROJOBJ)static-release
	tmake $(TMAKEPREHACK) "OBJECTS_DIR=.obj/$(PROJOBJ)static-release" $(PROJECT).pro -o GNUmakefile$(PROJMAK).static-release \
	    "CONFIG+=staticlib release" $(TMAKEHACK)

GNUmakefile$(PROJMAK).profile: $(PROJECT).pro GNUmakefile
ifeq ($(LIBTARGET),1)
	$(SYNCQT)
endif
	@mkdir -p .obj/$(PROJOBJ)profile
	tmake $(TMAKEPREHACK) "OBJECTS_DIR=.obj/$(PROJOBJ)profile" $(PROJECT).pro -o GNUmakefile$(PROJMAK).profile \
	    "CONFIG+=staticlib release" $(TMAKEHACK)
	perl -pi \
	    -e 's/^([CL]X*)FLAGS\s*=/$$1FLAGS=-g -pg /; s|-lqt|\$$(QTDIR)/lib$(PROJMAK)/libqt.a $(PROFLIBS) |' GNUmakefile$(PROJMAK).profile

GNUmakefile$(PROJMAK).gcc-debug: $(PROJECT).pro GNUmakefile
ifeq ($(LIBTARGET),1)
	$(SYNCQT)
endif
	@mkdir -p .obj/$(PROJOBJ)gcc-debug
	tmake $(TMAKEPREHACK) "OBJECTS_DIR=.obj/$(PROJOBJ)gcc-debug" $(PROJECT).pro -o GNUmakefile$(PROJMAK).gcc-debug \
	    "CONFIG+=debug" "TMAKE_CXX=gcc" $(TMAKEHACK)

GNUmakefile$(PROJMAK).gcc-release: $(PROJECT).pro GNUmakefile
ifeq ($(LIBTARGET),1)
	$(SYNCQT)
endif
	@mkdir -p .obj/$(PROJOBJ)gcc-release
	tmake $(TMAKEPREHACK) "OBJECTS_DIR=.obj/$(PROJOBJ)gcc-release" $(PROJECT).pro -o GNUmakefile$(PROJMAK).gcc-release \
	    "CONFIG+=release" "TMAKE_CXX=gcc"

GNUmakefile$(PROJMAK).gcc-static-debug: $(PROJECT).pro GNUmakefile
ifeq ($(LIBTARGET),1)
	$(SYNCQT)
endif
	@mkdir -p .obj/$(PROJOBJ)gcc-static-debug
	tmake $(TMAKEPREHACK) "OBJECTS_DIR=.obj/$(PROJOBJ)gcc-static-debug" $(PROJECT).pro -o GNUmakefile$(PROJMAK).gcc-static-debug \
	    "CONFIG+=staticlib debug" "TMAKE_CXX=gcc"

GNUmakefile$(PROJMAK).gcc-static-release: $(PROJECT).pro GNUmakefile
ifeq ($(LIBTARGET),1)
	$(SYNCQT)
endif
	@mkdir -p .obj/$(PROJOBJ)gcc-static-release
	tmake $(TMAKEPREHACK) "OBJECTS_DIR=.obj/$(PROJOBJ)gcc-static-release" $(PROJECT).pro -o GNUmakefile$(PROJMAK).gcc-static-release \
	    "CONFIG+=staticlib release" "TMAKE_CXX=gcc" $(TMAKEHACK)



showfiles:
	@tmake $(TMAKEPREHACK) $(PROJECT).pro -e 'Expand("HEADERS","SOURCES")'

showheaders:
	@tmake $(TMAKEPREHACK) $(PROJECT).pro -e 'Expand("HEADERS")'

showsources:
	@tmake $(TMAKEPREHACK) $(PROJECT).pro -e 'Expand("SOURCES")'

tags: $(PROJECT).pro
	@tmake $(TMAKEPREHACK) $(PROJECT).pro -e 'Expand("HEADERS","SOURCES")' \
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

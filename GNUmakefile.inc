#############################################################################
# $Id: //depot/qt/main/GNUmakefile.inc#11 $
#
# Unix Makefile, requires GNU make (gmake).
#
# Copyright (C) 1998 by Troll Tech AS.  All rights reserved.
#
#############################################################################

ifndef RELEASE
RELEASE=0
endif
ifeq ($(RELEASE),0)
all: debug
else
all: release
endif

ifndef LIBTARGET
LIBTARGET = 0
endif
ifeq ($(LIBTARGET),0)
VCTEMPLATE = vcapp
else
VCTEMPLATE = vclib
endif

project: $(PROJECT).pro

$(PROJECT).pro:
	progen -o "$(PROJECT).pro" -n "$(PROJECT)"


%: GNUmakefile.% $(MORETARGETS)
	$(MAKE) -j4 -f GNUmakefile.$*

static: static-debug

gcc: gcc-debug

GNUmakefile.debug: $(PROJECT).pro GNUmakefile
ifeq ($(LIBTARGET),1)
	syncqt
endif
	@mkdir -p .obj/debug
	tmake $(PROJECT).pro -o GNUmakefile.debug \
	    "CONFIG+=debug" "OBJECTS_DIR=.obj/debug"

GNUmakefile.release: $(PROJECT).pro GNUmakefile
ifeq ($(LIBTARGET),1)
	syncqt
endif
	@mkdir -p .obj/release
	tmake $(PROJECT).pro -o GNUmakefile.release \
	    "CONFIG+=release" "OBJECTS_DIR=.obj/release"

GNUmakefile.release-debug: $(PROJECT).pro GNUmakefile
ifeq ($(LIBTARGET),1)
	syncqt
endif
	@mkdir -p .obj/release-debug
	tmake $(PROJECT).pro -o GNUmakefile.release-debug \
	    "CONFIG+=release" "OBJECTS_DIR=.obj/release-debug"
	perl -pi \
	    -e 's/^([CL]X*)FLAGS\s*=/$$1FLAGS=-g /' \
		GNUmakefile.release-debug

GNUmakefile.static-debug: $(PROJECT).pro GNUmakefile
ifeq ($(LIBTARGET),1)
	syncqt
endif
	@mkdir -p .obj/static-debug
	tmake $(PROJECT).pro -o GNUmakefile.static-debug \
	    "CONFIG+=staticlib debug" "OBJECTS_DIR=.obj/static-debug"

GNUmakefile.static-release: $(PROJECT).pro GNUmakefile
ifeq ($(LIBTARGET),1)
	syncqt
endif
	@mkdir -p .obj/static-release
	tmake $(PROJECT).pro -o GNUmakefile.static-release \
	    "CONFIG+=staticlib release" "OBJECTS_DIR=.obj/static-release"

GNUmakefile.profile: $(PROJECT).pro GNUmakefile
ifeq ($(LIBTARGET),1)
	syncqt
endif
	@mkdir -p .obj/profile
	tmake $(PROJECT).pro -o GNUmakefile.profile \
	    "CONFIG+=staticlib release" "OBJECTS_DIR=.obj/profile"
	perl -pi \
	    -e 's/^([CL]X*)FLAGS\s*=/$$1FLAGS=-pg /; s|-lqt|\$$(QTDIR)/lib/libqt.a|' \
		GNUmakefile.profile

GNUmakefile.gcc-debug: $(PROJECT).pro GNUmakefile
ifeq ($(LIBTARGET),1)
	syncqt
endif
	@mkdir -p .obj/gcc-debug
	tmake $(PROJECT).pro -o GNUmakefile.gcc-debug \
	    "CONFIG+=debug" "TMAKE_CXX=gcc" "OBJECTS_DIR=.obj/gcc-debug"

GNUmakefile.gcc-release: $(PROJECT).pro GNUmakefile
ifeq ($(LIBTARGET),1)
	syncqt
endif
	@mkdir -p .obj/gcc-release
	tmake $(PROJECT).pro -o GNUmakefile.gcc-release \
	    "CONFIG+=release" "TMAKE_CXX=gcc" "OBJECTS_DIR=.obj/gcc-release"

GNUmakefile.gcc-static-debug: $(PROJECT).pro GNUmakefile
ifeq ($(LIBTARGET),1)
	syncqt
endif
	@mkdir -p .obj/gcc-static-debug
	tmake $(PROJECT).pro -o GNUmakefile.gcc-static-debug \
	    "CONFIG+=staticlib debug" "TMAKE_CXX=gcc" "OBJECTS_DIR=.obj/gcc-static-debug"

GNUmakefile.gcc-static-release: $(PROJECT).pro GNUmakefile
ifeq ($(LIBTARGET),1)
	syncqt
endif
	@mkdir -p .obj/gcc-static-release
	tmake $(PROJECT).pro -o GNUmakefile.gcc-static-release \
	    "CONFIG+=staticlib release" "TMAKE_CXX=gcc" "OBJECTS_DIR=.obj/gcc-static-release"


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
	syncqt

update:
	cvs update -d
	$(MAKE) depend

updateqt:
	$(MAKE) -C $(QTDIR)/src update

depend:
	-rm -f GNUmakefile.*debug GNUmakefile.*release GNUmakefile.profile $(MOREDEP)

clean: GNUmakefile.debug
	$(MAKE) -f GNUmakefile.debug clean
	-rm -f GNUmakefile.*debug GNUmakefile.*release GNUmakefile.profile $(MOREDEP) $(MORECLEAN)
	-rm -f core *~
	-rm -r -f .obj

help:
	@cat $(QTDIR)/make.help

win windows:
	nmake $(MAKEFLAGS)

bwin:
	bmake $(MAKEFLAGS)

FORCE:

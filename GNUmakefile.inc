#############################################################################
# $Id: //depot/qt/main/GNUmakefile.inc#5 $
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

static: GNUmakefile.static $(MORETARGETS)
	$(MAKE) -f GNUmakefile.static

gcc-debug: GNUmakefile.gcc-debug $(MORETARGETS)
	$(MAKE) -f GNUmakefile.gcc-debug

gcc-static: GNUmakefile.gcc-static $(MORETARGETS)
	$(MAKE) -f GNUmakefile.gcc-static

debug: GNUmakefile.debug $(MORETARGETS)
	$(MAKE) -f GNUmakefile.debug

release: GNUmakefile.release $(MORETARGETS)
	$(MAKE) -f GNUmakefile.release

GNUmakefile.gcc-static: $(PROJECT).pro GNUmakefile
ifeq ($(LIBTARGET),1)
	syncqt
endif
	@mkdir -p .obj/gcc-static
	tmake $(PROJECT).pro -o GNUmakefile.gcc-static \
	    "CONFIG+=staticlib" "TMAKE_CC=gcc" "OBJECTS_DIR=.obj/gcc-static"

GNUmakefile.gcc-debug: $(PROJECT).pro GNUmakefile
ifeq ($(LIBTARGET),1)
	syncqt
endif
	@mkdir -p .obj/gcc-debug
	tmake $(PROJECT).pro -o GNUmakefile.gcc-debug \
	    "TMAKE_CC=gcc" "OBJECTS_DIR=.obj/gcc-debug"

GNUmakefile.static: $(PROJECT).pro GNUmakefile
ifeq ($(LIBTARGET),1)
	syncqt
endif
	@mkdir -p .obj/static
	tmake $(PROJECT).pro -o GNUmakefile.static \
	    "CONFIG+=staticlib" "OBJECTS_DIR=.obj/static"

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
	-rm -f GNUmakefile.debug GNUmakefile.release $(MOREDEP)

clean: GNUmakefile.debug
	$(MAKE) -f GNUmakefile.debug clean
	-rm -f GNUmakefile.debug GNUmakefile.release GNUmakefile.static $(MOREDEP) $(MORECLEAN)
	-rm -f core *~
	-rm -r -f .obj

help:
	@cat $(QTDIR)/make.help

FORCE:

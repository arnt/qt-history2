#############################################################################
# $Id: //depot/qt/main/GNUmakefile.inc#1 $
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

debug: GNUmakefile.debug $(MORETARGETS)
	$(MAKE) -f GNUmakefile.debug

release: GNUmakefile.release $(MORETARGETS)
	$(MAKE) -f GNUmakefile.release

GNUmakefile.debug: $(PROJECT).pro GNUmakefile
	syncqt
	@mkdir -p .obj/debug
	tmake $(PROJECT).pro -o GNUmakefile.debug \
	    "CONFIG+=debug" "OBJECTS_DIR=.obj/debug"

GNUmakefile.release: $(PROJECT).pro GNUmakefile
	syncqt
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
	-rm -f $(PROJECT).pro GNUmakefile.debug GNUmakefile.release $(MOREDEP) $(MORECLEAN)
	-rm -f core *~
	-rm -r -f .obj

help:
	@cat $(QTDIR)/make.help

FORCE:

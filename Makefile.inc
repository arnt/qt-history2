#############################################################################
# $Id: //depot/qt/main/Makefile.inc#3 $
#
# Win32 Makefile, requires Microsoft nmake.
#
# Copyright (C) 1998 by Troll Tech AS.  All rights reserved.
#
#############################################################################

!IFNDEF RELEASE
RELEASE = 0
!ENDIF
!IF $(RELEASE) == 0
all: debug
!ELSE
all: release
!ENDIF

!IFNDEF LIBTARGET
LIBTARGET = 0
!ENDIF
!IF $(LIBTARGET) == 0
VCTEMPLATE = vcapp
!ELSE
VCTEMPLATE = vclib
!ENDIF

project: $(PROJECT).pro

$(PROJECT).pro:
	progen -o "$(PROJECT).pro" -n "$(PROJECT)"

dsp: $(PROJECT).dsp

$(PROJECT).dsp: FORCE
	tmake $(PROJECT).pro -t $(VCTEMPLATE) -o $(PROJECT).dsp

debug: Makefile.debug $(MORETARGETS) $(MORETARGETS_DEBUG)
	$(MAKE) -nologo -f Makefile.debug
	$(POST_DBG)

release: Makefile.release $(MORETARGETS) $(MORETARGETS_RELEASE)
	$(MAKE) -nologo -f Makefile.release
	$(POST_REL)

Makefile.debug: $(PROJECT).pro Makefile
!IF $(LIBTARGET) == 1
	syncqt
!ENDIF
	-@mkdir .obj\debug
	tmake $(PROJECT).pro -o Makefile.debug \
	    "CONFIG+=debug" "OBJECTS_DIR=.obj/debug"

Makefile.release: $(PROJECT).pro Makefile
!IF $(LIBTARGET) == 1
	syncqt
!ENDIF
	-@mkdir .obj\release
	tmake $(PROJECT).pro -o Makefile.release \
	    "CONFIG+=release" "OBJECTS_DIR=.obj/release"


showfiles:
	@tmake $(PROJECT).pro -e 'Expand("HEADERS","SOURCES")'

showheaders:
	@tmake $(PROJECT).pro -e 'Expand("HEADERS")'

showsources:
	@tmake $(PROJECT).pro -e 'Expand("SOURCES")'


include:
	syncqt

update:
	cvs update -d
	$(MAKE) depend

updateqt:
	cd $(QTDIR)\src
	$(MAKE) update

depend:
	-del Makefile.debug
	-del Makefile.release
!IFDEF MOREDEP
	-del $(MOREDEP)
!ENDIF

clean: Makefile.debug
	$(MAKE) -nologo -f Makefile.debug clean
	-del Makefile.debug
	-del Makefile.release
	-del vc*.pdb
	-del $(PROJECT).pdb
	-del $(PROJECT).ilk
	-del $(PROJECT).exe
	-del /Q .obj\debug\*
	-del /Q .obj\release\*
	-rmdir .obj\debug
	-rmdir .obj\release
	-rmdir .obj
!IFDEF MOREDEP
	-del $(MOREDEP)
!ENDIF
!IFDEF MORECLEAN
	-del $(MORECLEAN)
!ENDIF

cleandsp:
	$(MAKE) -nologo clean
	-del $(PROJECT).dsp
	-del $(PROJECT).dsw
	-del $(PROJECT).ncb
	-del $(PROJECT).opt
	-del $(PROJECT).plg
	-del /Q Debug\*
	-del /Q Release\*
	-rmdir Debug
	-rmdir Release

help:
	@type $(QTDIR)\make.help

FORCE:

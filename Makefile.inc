#############################################################################
# $Id: //depot/qt/main/Makefile.inc#5 $
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

debug: NMakefile.debug $(MORETARGETS) $(MORETARGETS_DEBUG)
	$(MAKE) -nologo -f NMakefile.debug

release: NMakefile.release $(MORETARGETS) $(MORETARGETS_RELEASE)
	$(MAKE) -nologo -f NMakefile.release

static-debug: NMakefile.static-debug $(MORETARGETS) $(MORETARGETS_DEBUG)
	$(MAKE) -nologo -f NMakefile.static-debug

static-release: NMakefile.static-release $(MORETARGETS) $(MORETARGETS_RELEASE)
	$(MAKE) -nologo -f NMakefile.static-release

NMakefile.debug: $(PROJECT).pro Makefile
!IF $(LIBTARGET) == 1
	syncqt
!ENDIF
	-@mkdir .obj
	-@mkdir .obj\debug
!IF "$(PROJECT)" == "qt"
	tmake $(PROJECT).pro -o NMakefile.debug \
	    "CONFIG+=debug" "DEFINES+=QT_MAKEDLL" "OBJECTS_DIR=.obj/debug"
!ELSE
	tmake $(PROJECT).pro -o NMakefile.debug \
	    "CONFIG+=debug" "DEFINES+=QT_DLL" "OBJECTS_DIR=.obj/debug"
!ENDIF

NMakefile.release: $(PROJECT).pro Makefile
!IF $(LIBTARGET) == 1
	syncqt
!ENDIF
	-@mkdir .obj
	-@mkdir .obj\release
!IF "$(PROJECT)" == "qt"
	tmake $(PROJECT).pro -o NMakefile.release \
	    "CONFIG+=release" "DEFINES+=QT_MAKEDLL" "OBJECTS_DIR=.obj/release"
!ELSE
	tmake $(PROJECT).pro -o NMakefile.release \
	    "CONFIG+=release" "DEFINES+=QT_DLL" "OBJECTS_DIR=.obj/release"
!ENDIF

NMakefile.static-debug: $(PROJECT).pro Makefile
!IF $(LIBTARGET) == 1
	syncqt
!ENDIF
	-@mkdir .obj
	-@mkdir .obj\static-debug
	tmake $(PROJECT).pro -o NMakefile.static-debug \
	    "CONFIG+=debug" "DEFINES+=QT_NODLL" "OBJECTS_DIR=.obj/static-debug"

NMakefile.static-release: $(PROJECT).pro Makefile
!IF $(LIBTARGET) == 1
	syncqt
!ENDIF
	-@mkdir .obj
	-@mkdir .obj\static-release
	tmake $(PROJECT).pro -o NMakefile.static-release \
	    "CONFIG+=release" "DEFINES+=QT_NODLL" "OBJECTS_DIR=.obj/static-release"


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
	-del NMakefile.*
!IFDEF MOREDEP
	-del $(MOREDEP)
!ENDIF

clean: NMakefile.debug
	$(MAKE) -nologo -f NMakefile.debug clean
	-del NMakefile.*
	-del vc*.pdb
	-del $(PROJECT).pdb
	-del $(PROJECT).ilk
	-del $(PROJECT).exe
	-del /Q .obj\debug\*
	-del /Q .obj\release\*
	-del /Q .obj\static-debug\*
	-del /Q .obj\static-release\*
	-rmdir .obj\debug
	-rmdir .obj\release
	-rmdir .obj\static-debug
	-rmdir .obj\static-release
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

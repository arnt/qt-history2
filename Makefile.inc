#############################################################################
# $Id: //depot/qt/main/Makefile.inc#15 $
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
	tmake $(PROJECT).pro "CONFIG+=debug" "DEFINES+=QT_DLL" -t $(VCTEMPLATE) -o $(PROJECT).dsp


debug: NMakefile.debug $(MORETARGETS) $(MORETARGETS_DEBUG)
	$(MAKE) /$(MAKEFLAGS) -nologo -f NMakefile.debug

release: NMakefile.release $(MORETARGETS) $(MORETARGETS_RELEASE)
	$(MAKE) /$(MAKEFLAGS) -nologo -f NMakefile.release

static-debug: NMakefile.static-debug $(MORETARGETS) $(MORETARGETS_DEBUG)
	$(MAKE) /$(MAKEFLAGS) -nologo -f NMakefile.static-debug

static-release: NMakefile.static-release $(MORETARGETS) $(MORETARGETS_RELEASE)
	$(MAKE) /$(MAKEFLAGS) -nologo -f NMakefile.static-release

borland: borland-debug

borland-debug: BMakefile.debug $(MORETARGETS) $(MORETARGETS_DEBUG)
	bmake -f BMakefile.debug

borland-release: BMakefile.release $(MORETARGETS) $(MORETARGETS_DEBUG)
	bmake -f BMakefile.release

borland-staticdebug: BMakefile.staticdebug $(MORETARGETS) $(MORETARGETS_DEBUG)
	bmake -f BMakefile.staticdebug

borland-staticrelease: BMakefile.staticrelease $(MORETARGETS) $(MORETARGETS_DEBUG)
	bmake -f BMakefile.staticrelease

x11: x11-debug

x11-debug: NMakefile.x11-debug $(MORETARGETS) $(MORETARGETS_DEBUG)
	$(MAKE) /$(MAKEFLAGS) -nologo -f NMakefile.x11-debug

x11-release: NMakefile.x11-release $(MORETARGETS) $(MORETARGETS_RELEASE)
	$(MAKE) /$(MAKEFLAGS) -nologo -f NMakefile.x11-release

NMakefile.debug: $(PROJECT).pro Makefile
!IF $(LIBTARGET) == 1
	syncqt
!ENDIF
!IF "$(PROJECT)" == "qt"
	tmake $(PROJECT).pro -o NMakefile.debug \
	    "CONFIG+=debug console" "DEFINES+=QT_MAKEDLL" "OBJECTS_DIR=.obj/debug"
!ELSE
	tmake $(PROJECT).pro -o NMakefile.debug \
	    "CONFIG+=debug console" "DEFINES+=QT_DLL" "OBJECTS_DIR=.obj/debug"
!ENDIF

NMakefile.release: $(PROJECT).pro Makefile
!IF $(LIBTARGET) == 1
	syncqt
!ENDIF
!IF "$(PROJECT)" == "qt"
	tmake $(PROJECT).pro -o NMakefile.release \
	    "CONFIG+=release" "DEFINES+=QT_MAKEDLL" "OBJECTS_DIR=.obj/release" "TMAKE_LFLAGS_RELEASE=/INCREMENTAL:YES"
!ELSE
	tmake $(PROJECT).pro -o NMakefile.release \
	    "CONFIG+=release" "DEFINES+=QT_DLL" "OBJECTS_DIR=.obj/release" "TMAKE_LFLAGS_RELEASE=/INCREMENTAL:YES"
!ENDIF

NMakefile.static-debug: $(PROJECT).pro Makefile
!IF $(LIBTARGET) == 1
	syncqt
!ENDIF
	tmake $(PROJECT).pro -o NMakefile.static-debug \
	    "CONFIG+=debug console" "DEFINES+=QT_NODLL" "OBJECTS_DIR=.obj/static-debug"

NMakefile.static-release: $(PROJECT).pro Makefile
!IF $(LIBTARGET) == 1
	syncqt
!ENDIF
	tmake $(PROJECT).pro -o NMakefile.static-release \
	    "CONFIG+=release" "DEFINES+=QT_NODLL" "OBJECTS_DIR=.obj/static-release"

BMakefile.debug: $(PROJECT).pro Makefile
!IF $(LIBTARGET) == 1
	syncqt
!ENDIF
!IF "$(PROJECT)" =="qt"
	tmake "TMAKEPATH=/tmake/lib/win32-borland" \
	    $(PROJECT).pro -o BMakefile.debug \
	    "CONFIG+=debug" "DEFINES+=QT_MAKEDLL" "OBJECTS_DIR=borland/debug"
!ELSE
	tmake "TMAKEPATH=/tmake/lib/win32-borland" \
	    $(PROJECT).pro -o BMakefile.debug \
	    "CONFIG+=debug" "DEFINES+=QT_DLL" "OBJECTS_DIR=borland/debug"
!ENDIF

BMakefile.release: $(PROJECT).pro Makefile
!IF $(LIBTARGET) == 1
	syncqt
!ENDIF
!IF "$(PROJECT)" == "qt"
	tmake "TMAKEPATH=/tmake/lib/win32-borland" \
	    $(PROJECT).pro -o BMakefile.release \
	    "CONFIG+=release" "DEFINES+=QT_MAKEDLL" "OBJECTS_DIR=borland/release" "TMAKE_LFLAGS_RELEASE=/INCREMENTAL:YES"
!ELSE
	tmake "TMAKEPATH=/tmake/lib/win32-borland" \
	    $(PROJECT).pro -o BMakefile.release \
	    "CONFIG+=release" "DEFINES+=QT_DLL" "OBJECTS_DIR=borland/release" "TMAKE_LFLAGS_RELEASE=/INCREMENTAL:YES"
!ENDIF

BMakefile.staticdebug: $(PROJECT).pro Makefile
!IF $(LIBTARGET) == 1
	syncqt
!ENDIF
	tmake "TMAKEPATH=/tmake/lib/win32-borland" \
	    $(PROJECT).pro -o BMakefile.staticdebug \
	    "CONFIG+=debug" "DEFINES+=QT_NODLL" "OBJECTS_DIR=borland/staticdebug"

BMakefile.staticrelease: $(PROJECT).pro Makefile
!IF $(LIBTARGET) == 1
	syncqt
!ENDIF
	tmake "TMAKEPATH=/tmake/lib/win32-borland" \
	    $(PROJECT).pro -o BMakefile.staticrelease \
	    "CONFIG+=release" "DEFINES+=QT_NODLL" "OBJECTS_DIR=borland/staticrelease"

!IF "$(PROJECT)" == "qt"
NMakefile.x11-debug: qtx11.pro $(PROJECT).pro Makefile
	syncqt
	tmake qtx11.pro -o NMakefile.x11-debug \
	    "CONFIG=qt debug console" \
	    "DEFINES+=QT_NODLL WIN32 _WIN32_X11_" \
	    "OBJECTS_DIR=.obj/x11-debug" \
	    "INCLUDEPATH=$(X11R6)\include"

!ELSE
NMakefile.x11-debug: $(PROJECT).pro Makefile
	tmake $(PROJECT).pro -o NMakefile.x11-debug \
	    "CONFIG=qt debug console" \
	    "DEFINES+=QT_NODLL WIN32 _WIN32_X11_" \
	    "OBJECTS_DIR=.obj/x11-debug" \
	    "TMAKE_LIBS_QT/=s/qt/qtx11/" \
	    "TMAKE_LIBS_WINDOWS=user32.lib wsock32.lib" \
	    "TMAKE_LIBS=$(X11R6)\lib\X11.lib $(X11R6)\lib\Xext.lib"
!ENDIF

!IF "$(PROJECT)" == "qt"
NMakefile.x11-release: qtx11.pro $(PROJECT).pro Makefile
	syncqt
	tmake qtx11.pro -o NMakefile.x11-release \
	    "CONFIG=qt release" \
	    "DEFINES+=QT_NODLL WIN32 _WIN32_X11_" \
	    "OBJECTS_DIR=.obj/x11-release" \
	    "INCLUDEPATH=$(X11R6)\include"

!ELSE
NMakefile.x11-release: $(PROJECT).pro Makefile
	tmake $(PROJECT).pro -o NMakefile.x11-release \
	    "CONFIG=qt release" \
	    "DEFINES+=QT_NODLL WIN32 _WIN32_X11_" \
	    "OBJECTS_DIR=.obj/x11-release" \
	    "TMAKE_LIBS_QT/=s/qt/qtx11/" \
	    "TMAKE_LIBS_WINDOWS=user32.lib wsock32.lib" \
	    "TMAKE_LIBS=$(X11R6)\lib\X11.lib $(X11R6)\lib\Xext.lib"
!ENDIF


qtx11.pro: qt.pro qtinternal.pro
	@echo Building qtx11.pro...
	@echo # Generated project file for building Qt/X11 on Windows > qtx11.pro
	@echo TEMPLATE = lib >> qtx11.pro
	@echo MOC_DIR = tmp >> qtx11.pro
	@tmake -unix qt.pro qtinternal.pro -e "print 'HEADERS = '; Expand('HEADERS')" >> qtx11.pro
	@tmake -unix qt.pro qtinternal.pro -e "print 'SOURCES = '; Expand('SOURCES')" >> qtx11.pro
	@echo TARGET = qtx11 >> qtx11.pro
	@echo DESTDIR = ../lib >> qtx11.pro
	@echo DLLDESTDIR = ../bin >> qtx11.pro


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
	$(MAKE) /$(MAKEFLAGS) depend

updateqt:
	cd $(QTDIR)\src
	$(MAKE) /$(MAKEFLAGS) update

depend:
	-del NMakefile.*
	-del BMakefile.*
!IFDEF MOREDEP
	-del $(MOREDEP)
!ENDIF

clean: NMakefile.debug
	$(MAKE) /$(MAKEFLAGS) -nologo -f NMakefile.debug clean
	-del NMakefile.*
	-del BMakefile.*
	-del vc*.pdb
	-del $(PROJECT).pdb
	-del $(PROJECT).ilk
	-del $(PROJECT).exe
	-del /Q .obj\debug\*
	-del /Q .obj\release\*
	-del /Q .obj\static-debug\*
	-del /Q .obj\static-release\*
	-del /Q .obj\x11-debug\*
	-del /Q .obj\x11-release\*
	-rmdir .obj\debug
	-rmdir .obj\release
	-rmdir .obj\static-debug
	-rmdir .obj\static-release
	-rmdir .obj\X11-debug
	-rmdir .obj\x11-release
	-rmdir .obj
	-del BMakefile.*
	-del /Q borland\debug\*
	-del /Q borland\release\*
	-del /Q borland\staticdebug\*
	-del /Q borland\staticrelease\*
	-rmdir borland\debug
	-rmdir borland\release
	-rmdir borland\staticdebug
	-rmdir borland\staticrelease
	-rmdir borland
!IFDEF MOREDEP
	-del $(MOREDEP)
!ENDIF
!IFDEF MORECLEAN
	-del $(MORECLEAN)
!ENDIF

cleandsp:
	$(MAKE) /$(MAKEFLAGS) -nologo clean
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

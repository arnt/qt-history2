#############################################################################
# $Id: //depot/qt/main/Makefile.inc#18 $
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

!IF "$(MAKE)" == "NMAKE" || "$(MAKE)" == "Nmake" || "$(MAKE)" == "nmake"
MFILE = NMakefile
ODIR = obj\msvc
TPATH = $(TMAKEPATH)\..\win32-msvc
CONSOLE = console
!ELSE
MFILE = BMakefile
ODIR = obj\borland
TPATH = $(TMAKEPATH)\..\win32-borland
CONSOLE = 
!ENDIF

project: $(PROJECT).pro

$(PROJECT).pro:
	progen -o "$(PROJECT).pro" -n "$(PROJECT)"


dsp: $(PROJECT).dsp

$(PROJECT).dsp: FORCE
	tmake $(PROJECT).pro "CONFIG+=debug" "DEFINES+=QT_DLL" -t $(VCTEMPLATE) -o $(PROJECT).dsp


debug: $(MFILE).debug $(MORETARGETS) $(MORETARGETS_DEBUG)
	$(MAKE) /$(MAKEFLAGS) -f $(MFILE).debug

release: $(MFILE).release $(MORETARGETS) $(MORETARGETS_RELEASE)
	$(MAKE) /$(MAKEFLAGS) -f $(MFILE).release

static-debug: $(MFILE).static-debug $(MORETARGETS) $(MORETARGETS_DEBUG)
	$(MAKE) /$(MAKEFLAGS) -f $(MFILE).static-debug

static-release: $(MFILE).static-release $(MORETARGETS) $(MORETARGETS_RELEASE)
	$(MAKE) /$(MAKEFLAGS) -f $(MFILE).static-release



$(MFILE).debug: $(PROJECT).pro Makefile
!IF $(LIBTARGET) == 1
	syncqt
!ENDIF
!IF "$(PROJECT)" == "qt"
	tmake "TMAKEPATH=$(TPATH)" $(PROJECT).pro -o $(MFILE).debug \
		"CONFIG+=debug" "DEFINES+=QT_MAKEDLL" \
		"OBJECTS_DIR=$(ODIR)\debug"
!ELSE
	tmake "TMAKEPATH=$(TPATH)" $(PROJECT).pro -o $(MFILE).debug \
		"CONFIG+=debug $(CONSOLE)" "DEFINES+=QT_DLL" \
		"OBJECTS_DIR=$(ODIR)\debug"
!ENDIF


$(MFILE).release: $(PROJECT).pro Makefile
!IF $(LIBTARGET) == 1
	syncqt
!ENDIF
!IF "$(PROJECT)" == "qt"
	tmake "TMAKEPATH=$(TPATH)" $(PROJECT).pro -o $(MFILE).release \
		"CONFIG+=release" "DEFINES+=QT_MAKEDLL" \
		"OBJECTS_DIR=$(ODIR)\release" \
		"TMAKE_LFLAGS_RELEASE=/INCREMENTAL:YES"
!ELSE
	tmake "TMAKEPATH=$(TPATH)" $(PROJECT).pro -o $(MFILE).release \
		"CONFIG+=release" "DEFINES+=QT_DLL" \
		"OBJECTS_DIR=$(ODIR)\release" \
		"TMAKE_LFLAGS_RELEASE=/INCREMENTAL:YES"
!ENDIF



$(MFILE).static-debug: $(PROJECT).pro Makefile
!IF $(LIBTARGET) == 1
	syncqt
!ENDIF
	tmake "TMAKEPATH=$(TPATH)" $(PROJECT).pro -o $(MFILE).static-debug \
		"CONFIG+=debug console" "DEFINES+=QT_NODLL" \
		"OBJECTS_DIR=$(ODIR)\static_debug"


$(MFILE).static-release: $(PROJECT).pro Makefile
!IF $(LIBTARGET) == 1
	syncqt
!ENDIF
	tmake "TMAKEPATH=$(TPATH)" $(PROJECT).pro -o $(MFILE).static-release \
		"CONFIG+=release" "DEFINES+=QT_NODLL" \
		"OBJECTS_DIR=$(ODIR)\static_release"



x11: x11-debug

x11-debug: NMakefile.x11-debug $(MORETARGETS) $(MORETARGETS_DEBUG)
	$(MAKE) /$(MAKEFLAGS) -nologo -f NMakefile.x11-debug

x11-release: NMakefile.x11-release $(MORETARGETS) $(MORETARGETS_RELEASE)
	$(MAKE) /$(MAKEFLAGS) -nologo -f NMakefile.x11-release


!IF "$(PROJECT)" == "qt"
NMakefile.x11-debug: qtx11.pro $(PROJECT).pro Makefile
	syncqt
	tmake qtx11.pro -o NMakefile.x11-debug \
	    "CONFIG=qt debug console" \
	    "DEFINES+=QT_NODLL WIN32 _WIN32_X11_" \
	    "OBJECTS_DIR=obj\x11-debug" \
	    "INCLUDEPATH=$(X11R6)\include"

!ELSE
NMakefile.x11-debug: $(PROJECT).pro Makefile
	tmake $(PROJECT).pro -o NMakefile.x11-debug \
	    "CONFIG=qt debug console" \
	    "DEFINES+=QT_NODLL WIN32 _WIN32_X11_" \
	    "OBJECTS_DIR=obj\x11-debug" \
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
	    "OBJECTS_DIR=obj\x11-release" \
	    "INCLUDEPATH=$(X11R6)\include"

!ELSE
NMakefile.x11-release: $(PROJECT).pro Makefile
	tmake $(PROJECT).pro -o NMakefile.x11-release \
	    "CONFIG=qt release" \
	    "DEFINES+=QT_NODLL WIN32 _WIN32_X11_" \
	    "OBJECTS_DIR=obj\x11-release" \
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

clean: $(MFILE).debug
	$(MAKE) /$(MAKEFLAGS) -nologo -f $(MFILE).debug clean
	-del NMakefile.*
	-del BMakefile.*
	-del vc*.pdb
	-del $(PROJECT).pdb
	-del $(PROJECT).ilk
	-del $(PROJECT).exe
	-del /Q obj\msvc\debug\*
	-del /Q obj\msvc\release\*
	-del /Q obj\msvc\static_debug\*
	-del /Q obj\msvc\static_release\*
	-rmdir obj\msvc\debug
	-rmdir obj\msvc\release
	-rmdir obj\msvc\static_debug
	-rmdir obj\msvc\static_release
	-rmdir obj\msvc
	-del /Q obj\borland\debug\*
	-del /Q obj\borland\release\*
	-del /Q obj\borland\static_debug\*
	-del /Q obj\borland\static_release\*
	-rmdir obj\borland\debug
	-rmdir obj\borland\release
	-rmdir obj\borland\static_debug
	-rmdir obj\borland\static_release
	-rmdir obj\borland
	-del /Q obj\x11-debug\*
	-del /Q obj\x11-release\*
	-rmdir obj\X11-debug
	-rmdir obj\x11-release
	-rmdir obj
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
	-del $(PROJECT).stt
	-del /Q Debug\*
	-del /Q Release\*
	-rmdir Debug
	-rmdir Release

help:
	@type $(QTDIR)\make.help

FORCE:

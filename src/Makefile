#############################################################################
# $Id: //depot/qt/main/src/Makefile#73 $
#
# Win32 Makefile, requires Microsoft nmake.
#
# Copyright (C) 1998 by Troll Tech AS.  All rights reserved.
#
#############################################################################

PROJECT     = qt
LIBTARGET   = 1
MORETARGETS = qtmain

!INCLUDE $(QTDIR)\Makefile.inc


!IF $(RELEASE) == 0
qtmain: qtmain-debug.mak
	$(MAKE) -f qtmain-debug.mak
!ELSE
qtmain: qtmain-release.mak
	$(MAKE) -f qtmain-release.mak
!ENDIF

qtmain-release.mak: qt.pro
	tmake qt.pro -o qtmain-release.mak "CONFIG+=release"

qtmain-debug.mak: qt.pro
	tmake qt.pro -o qtmain-debug.mak "CONFIG+=debug"

qtmain: qtmain.mak
	$(MAKE) -f qtmain.mak

qtmain.mak: qtmain.pro
	tmake qtmain.pro -o qtmain.mak


!IF $(RELEASE) == 0
qtdll: qtdll-debug.mak
	$(MAKE) -f qtdll-debug.mak
	-@copy ..\lib\qt*.dll ..\bin
!ELSE
qtdll: qtdll-release.mak
	$(MAKE) -f qtdll-release.mak
	-@copy ..\lib\qt*.dll ..\bin
!ENDIF

qtdll-release.mak: qt.pro
	tmake qt.pro -o qtdll-release.mak "CONFIG+=dll release" \
		"DEFINES+=QT_MAKEDLL"

qtdll-debug.mak: qt.pro
	tmake qt.pro -o qtdll-debug.mak "CONFIG+=dll debug" \
		"DEFINES+=QT_MAKEDLL"

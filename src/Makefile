#############################################################################
# $Id: //depot/qt/main/src/Makefile#74 $
#
# Win32 Makefile, requires Microsoft nmake.
#
# Copyright (C) 1998 by Troll Tech AS.  All rights reserved.
#
#############################################################################

PROJECT		    = qt
LIBTARGET	    = 1
MORETARGETS_RELEASE = qtmain-release
MORETARGETS_DEBUG   = qtmain-debug
MORECLEAN	    = qtmain.release.mak qtmain.debug.mak

!INCLUDE $(QTDIR)\Makefile.inc

!IF $(RELEASE) == 0
qtmain: qtmain-debug
!ELSE
qtmain: qtmain-release
!ENDIF

qtmain-debug: qtmain.debug.mak
	$(MAKE) -f qtmain.debug.mak

qtmain-release: qtmain.release.mak
	$(MAKE) -f qtmain.release.mak

qtmain.release.mak: qtmain.pro
	tmake qtmain.pro -o qtmain.release.mak \
		"CONFIG+=release" "OBJECTS_DIR=.obj/release"

qtmain.debug.mak: qtmain.pro
	tmake qtmain.pro -o qtmain.debug.mak \
		"CONFIG+=debug" "OBJECTS_DIR=.obj/debug"

#############################################################################
# $Id: //depot/qt/main/src/Makefile#87 $
#
# Win32 Makefile, requires Microsoft nmake.
#
# Copyright (C) 1999 by Troll Tech AS.  All rights reserved.
#
#############################################################################

PROJECT		    = qt
LIBTARGET	    = 1
MORETARGETS_RELEASE = qtmain-release
MORETARGETS_DEBUG   = qtmain-debug

!INCLUDE $(QTDIR)\Makefile.inc

!IF $(RELEASE) == 0
qtmain: qtmain-debug
!ELSE
qtmain: qtmain-release
!ENDIF

qtmain-debug: NMakefile.qtmain-debug
	$(MAKE) -f NMakefile.qtmain-debug

qtmain-release: NMakefile.qtmain-release
	$(MAKE) -f NMakefile.qtmain-release

NMakefile.qtmain-debug: qtmain.pro
	tmake qtmain.pro -o NMakefile.qtmain-debug \
		"CONFIG+=debug" "OBJECTS_DIR=.obj/debug"

NMakefile.qtmain-release: qtmain.pro
	tmake qtmain.pro -o NMakefile.qtmain-release \
		"CONFIG+=release" "OBJECTS_DIR=.obj/release"

qtmain-borland-debug: NMakefile.qtmain-borland-debug
	bmake -f NMakefile.qtmain-borland-debug

qtmain-borland-release: NMakefile.qtmain-borland-release
	bmake -f NMakefile.qtmain-borland-release

NMakefile.qtmain-borland-debug: qtmain.pro Makefile
	tmake "TMAKEPATH=\tmake\lib\win32-borland" \
		qtmain.pro -o NMakefile.qtmain-borland-debug \
		"CONFIG+=debug" "OBJECTS_DIR=borland/debug"

NMakefile.qtmain-borland-release: qtmain.pro Makefile
	tmake "TMAKEPATH=\tmake\lib\win32-borland" \
		qtmain.pro -o NMakefile.qtmain-borland-release \
		"CONFIG+=release" "OBJECTS_DIR=borland/release"

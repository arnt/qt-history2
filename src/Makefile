#############################################################################
# $Id: //depot/qt/main/src/Makefile#78 $
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
	-@mkdir .obj
	-@mkdir .obj\debug	
	tmake qtmain.pro -o NMakefile.qtmain-debug \
		"CONFIG+=debug" "OBJECTS_DIR=.obj/debug"

NMakefile.qtmain-release: qtmain.pro
	-@mkdir .obj
	-@mkdir .obj\release
	tmake qtmain.pro -o NMakefile.qtmain-release \
		"CONFIG+=release" "OBJECTS_DIR=.obj/release"

#############################################################################
# $Id: //depot/qt/main/src/Makefile#90 $
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

qtmain-debug: $(MFILE).qtmain-debug
	$(MAKE) -f $(MFILE).qtmain-debug

qtmain-release: $(MFILE).qtmain-release
	$(MAKE) -f $(MFILE).qtmain-release

$(MFILE).qtmain-debug: qtmain.pro
	tmake "TMAKEPATH=$(TPATH)" qtmain.pro -o $(MFILE).qtmain-debug \
		"CONFIG+=debug" "OBJECTS_DIR=$(ODIR)/debug"

$(MFILE).qtmain-release: qtmain.pro
	tmake "TMAKEPATH=$(TPATH)" qtmain.pro -o $(MFILE).qtmain-release \
		"CONFIG+=release" "OBJECTS_DIR=$(ODIR)/release"

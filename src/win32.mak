#############################################################################
# $Id: //depot/qt/main/src/win32.mak#1 $
#
# Win32 Makefile for building the Qt libraries.
#
# Copyright (C) 1998 by Troll Tech AS.  All rights reserved.
#
#############################################################################


all: qtstatic

qtstatic: qtmain
	$(MAKE) -f qtstatic.mak

qtdll: qtmain
	$(MAKE) -f qtdll.mak
	@copy ..\lib\qt*.dll ..\bin

qtmain:
	$(MAKE) -f qtmain.mak

clean:
	$(MAKE) -f qtdll.mak clean
	$(MAKE) -f qtmain.mak clean
	-@del ..\lib\qt*.dll
	-@del ..\lib\qt*.exp

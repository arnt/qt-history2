#############################################################################
# $Id: //depot/qt/main/util/qws/GNUmakefile#4 $
#
# Unix Makefile, requires GNU make (gmake).
#
# Copyright (C) 1998 by Trolltech AS.  All rights reserved.
#
#############################################################################

PROJECT = qws

include $(QTDIR)/GNUmakefile.inc

doc: regions.gif windows.gif

%.gif: %.fig
	fig2dev -L ppm $*.fig $*.ppm
	pnmcrop $*.ppm | ppmtogif >$*.gif

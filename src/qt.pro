TEMPLATE	=	lib
CONFIG		=	qt warn_on release
unix:CONFIG    +=	x11inc

win32:INCLUDEPATH =	tmp
win32:MOC_DIR     =	tmp

win32:DIALOGS_H	=	../include
win32:KERNEL_H	=	../include
win32:TOOLS_H	=	../include
win32:WIDGETS_H	=	../include

unix:DIALOGS_H	=	dialogs
unix:KERNEL_H	=	kernel
unix:TOOLS_H	=	tools
unix:WIDGETS_H	=	widgets

HEADERS     =	$$DIALOGS_H/qfiledlg.h \
		$$DIALOGS_H/qmsgbox.h \
		$$DIALOGS_H/qprogdlg.h \
		$$DIALOGS_H/qtabdlg.h \
		$$KERNEL_H/qaccel.h \
		$$KERNEL_H/qapp.h \
		$$KERNEL_H/qasyncimageio.h \
		$$KERNEL_H/qasyncio.h \
		$$KERNEL_H/qbitmap.h \
		$$KERNEL_H/qbrush.h \
		$$KERNEL_H/qclipbrd.h \
		$$KERNEL_H/qcolor.h \
		$$KERNEL_H/qconnect.h \
		$$KERNEL_H/qcursor.h \
		$$KERNEL_H/qdialog.h \
		$$KERNEL_H/qdrawutl.h \
		$$KERNEL_H/qevent.h \
		$$KERNEL_H/qfont.h \
		$$KERNEL_H/qfontdta.h \
		$$KERNEL_H/qfontinf.h \
		$$KERNEL_H/qfontmet.h \
		$$KERNEL_H/qgmanagr.h \
		$$KERNEL_H/qimage.h \
		$$KERNEL_H/qkeycode.h \
		$$KERNEL_H/qlayout.h \
		$$KERNEL_H/qmetaobj.h \
		$$KERNEL_H/qmovie.h \
		$$KERNEL_H/qobjcoll.h \
		$$KERNEL_H/qobjdefs.h \
		$$KERNEL_H/qobject.h \
		$$KERNEL_H/qpaintd.h \
		$$KERNEL_H/qpaintdc.h \
		$$KERNEL_H/qpainter.h \
		$$KERNEL_H/qpalette.h \
		$$KERNEL_H/qpdevmet.h \
		$$KERNEL_H/qpen.h \
		$$KERNEL_H/qpicture.h \
		$$KERNEL_H/qpixmap.h \
		$$KERNEL_H/qpmcache.h \
		$$KERNEL_H/qpntarry.h \
		$$KERNEL_H/qpoint.h \
		$$KERNEL_H/qprinter.h \
		$$KERNEL_H/qrect.h \
		$$KERNEL_H/qregion.h \
		$$KERNEL_H/qsemimodal.h \
		$$KERNEL_H/qsignal.h \
		$$KERNEL_H/qsize.h \
		$$KERNEL_H/qsocknot.h \
		$$KERNEL_H/qtimer.h \
		$$KERNEL_H/qwidcoll.h \
		$$KERNEL_H/qwidget.h \
		$$KERNEL_H/qwindefs.h \
		$$KERNEL_H/qwindow.h \
		$$KERNEL_H/qwmatrix.h \
		$$TOOLS_H/qarray.h \
		$$TOOLS_H/qbitarry.h \
		$$TOOLS_H/qbuffer.h \
		$$TOOLS_H/qcache.h \
		$$TOOLS_H/qcollect.h \
		$$TOOLS_H/qdatetm.h \
		$$TOOLS_H/qdict.h \
		$$TOOLS_H/qdir.h \
		$$TOOLS_H/qdstream.h \
		$$TOOLS_H/qfile.h \
		$$TOOLS_H/qfiledef.h \
		$$TOOLS_H/qfileinf.h \
		$$TOOLS_H/qgarray.h \
		$$TOOLS_H/qgcache.h \
		$$TOOLS_H/qgdict.h \
		$$TOOLS_H/qgeneric.h \
		$$TOOLS_H/qglist.h \
		$$TOOLS_H/qglobal.h \
		$$TOOLS_H/qgvector.h \
		$$TOOLS_H/qintcach.h \
		$$TOOLS_H/qintdict.h \
		$$TOOLS_H/qiodev.h \
		$$TOOLS_H/qlist.h \
		$$TOOLS_H/qptrdict.h \
		$$TOOLS_H/qqueue.h \
		$$TOOLS_H/qregexp.h \
		$$TOOLS_H/qshared.h \
		$$TOOLS_H/qstack.h \
		$$TOOLS_H/qstring.h \
		$$TOOLS_H/qstrlist.h \
		$$TOOLS_H/qstrvec.h \
		$$TOOLS_H/qtstream.h \
		$$TOOLS_H/qvector.h \
		$$WIDGETS_H/qbttngrp.h \
		$$WIDGETS_H/qbutton.h \
		$$WIDGETS_H/qchkbox.h \
		$$WIDGETS_H/qcombo.h \
		$$WIDGETS_H/qframe.h \
		$$WIDGETS_H/qgrpbox.h \
		$$WIDGETS_H/qheader.h \
		$$WIDGETS_H/qlabel.h \
		$$WIDGETS_H/qlcdnum.h \
		$$WIDGETS_H/qlined.h \
		$$WIDGETS_H/qlistbox.h \
		$$WIDGETS_H/qlistview.h \
		$$WIDGETS_H/qmenubar.h \
		$$WIDGETS_H/qmenudta.h \
		$$WIDGETS_H/qmlined.h \
		$$WIDGETS_H/qpopmenu.h \
		$$WIDGETS_H/qprogbar.h \
		$$WIDGETS_H/qpushbt.h \
		$$WIDGETS_H/qradiobt.h \
		$$WIDGETS_H/qrangect.h \
		$$WIDGETS_H/qscrbar.h \
		$$WIDGETS_H/qscrollview.h \
		$$WIDGETS_H/qslider.h \
		$$WIDGETS_H/qspinbox.h \
		$$WIDGETS_H/qtabbar.h \
		$$WIDGETS_H/qtablevw.h \
		$$WIDGETS_H/qtooltip.h \
		$$WIDGETS_H/qvalidator.h

SOURCES     =	dialogs/qfiledlg.cpp \
		dialogs/qmsgbox.cpp \
		dialogs/qprogdlg.cpp \
		dialogs/qtabdlg.cpp \
		kernel/qaccel.cpp \
		kernel/qapp.cpp \
		kernel/qasyncimageio.cpp \
		kernel/qasyncio.cpp \
		kernel/qbitmap.cpp \
		kernel/qclipbrd.cpp \
		kernel/qcolor.cpp \
		kernel/qconnect.cpp \
		kernel/qcursor.cpp \
		kernel/qdialog.cpp \
		kernel/qdrawutl.cpp \
		kernel/qevent.cpp \
		kernel/qfont.cpp \
		kernel/qgmanagr.cpp \
		kernel/qimage.cpp \
		kernel/qlayout.cpp \
		kernel/qmetaobj.cpp \
		kernel/qmovie.cpp \
		kernel/qobject.cpp \
		kernel/qpainter.cpp \
		kernel/qpalette.cpp \
		kernel/qpdevmet.cpp \
		kernel/qpicture.cpp \
		kernel/qpixmap.cpp \
		kernel/qpmcache.cpp \
		kernel/qpntarry.cpp \
		kernel/qpoint.cpp \
		kernel/qprinter.cpp \
		kernel/qrect.cpp \
		kernel/qregion.cpp \
		kernel/qsemimodal.cpp \
		kernel/qsignal.cpp \
		kernel/qsize.cpp \
		kernel/qsocknot.cpp \
		kernel/qtimer.cpp \
		kernel/qwidget.cpp \
		kernel/qwindow.cpp \
		kernel/qwmatrix.cpp \
		tools/qbitarry.cpp \
		tools/qbuffer.cpp \
		tools/qcollect.cpp \
		tools/qdatetm.cpp \
		tools/qdir.cpp \
		tools/qdstream.cpp \
		tools/qfile.cpp \
		tools/qfileinf.cpp \
		tools/qgarray.cpp \
		tools/qgcache.cpp \
		tools/qgdict.cpp \
		tools/qglist.cpp \
		tools/qglobal.cpp \
		tools/qgvector.cpp \
		tools/qiodev.cpp \
		tools/qregexp.cpp \
		tools/qstring.cpp \
		tools/qtstream.cpp \
		widgets/qbttngrp.cpp \
		widgets/qbutton.cpp \
		widgets/qchkbox.cpp \
		widgets/qcombo.cpp \
		widgets/qframe.cpp \
		widgets/qgrpbox.cpp \
		widgets/qheader.cpp \
		widgets/qlabel.cpp \
		widgets/qlcdnum.cpp \
		widgets/qlined.cpp \
		widgets/qlistbox.cpp \
		widgets/qlistview.cpp \
		widgets/qmenubar.cpp \
		widgets/qmenudta.cpp \
		widgets/qmlined.cpp \
		widgets/qpopmenu.cpp \
		widgets/qprogbar.cpp \
		widgets/qpushbt.cpp \
		widgets/qradiobt.cpp \
		widgets/qrangect.cpp \
		widgets/qscrbar.cpp \
		widgets/qscrollview.cpp \
		widgets/qslider.cpp \
		widgets/qspinbox.cpp \
		widgets/qtabbar.cpp \
		widgets/qtablevw.cpp \
		widgets/qtooltip.cpp \
		widgets/qvalidator.cpp

win32:SOURCES +=kernel/qapp_win.cpp \
		kernel/qclb_win.cpp \
		kernel/qcol_win.cpp \
		kernel/qcur_win.cpp \
		kernel/qfnt_win.cpp \
		kernel/qpic_win.cpp \
		kernel/qpm_win.cpp \
		kernel/qprn_win.cpp \
		kernel/qptd_win.cpp \
		kernel/qptr_win.cpp \
		kernel/qrgn_win.cpp \
		kernel/qwid_win.cpp

unix:HEADERS +=	$$DIALOGS_H/qprndlg.h \
		$$KERNEL_H/qpsprn.h

unix:SOURCES += kernel/qapp_x11.cpp \
		kernel/qclb_x11.cpp \
		kernel/qcol_x11.cpp \
		kernel/qcur_x11.cpp \
		kernel/qfnt_x11.cpp \
		kernel/qpic_x11.cpp \
		kernel/qpm_x11.cpp \
		kernel/qprn_x11.cpp \
		kernel/qptd_x11.cpp \
		kernel/qptr_x11.cpp \
		kernel/qrgn_x11.cpp \
		kernel/qwid_x11.cpp

unix:SOURCES += dialogs/qprndlg.cpp \
		kernel/qpsprn.cpp \
		kernel/qnpsupport.cpp \
		kernel/qt_x11.cpp

TARGET      =	qt
VERSION     =	1.31
DESTDIR     =	../lib

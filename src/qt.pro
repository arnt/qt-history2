TEMPLATE	= lib
CONFIG		+= qt warn_on release

unix:CONFIG += x11
embedded:CONFIG -= x11

# CONFIG += newrichtext
CONFIG += oldrichtext

# CONFIG += newx11font
CONFIG += oldx11font

TARGET		= qt
VERSION		= 3.0.0
DESTDIR		= $$TMAKE_LIBDIR_QT
DLLDESTDIR	= ../bin

# All extension modules are listed here
# This is duplicated in examples.pro
MODULES_BASE	= tools kernel widgets dialogs
MODULES_PRO	= iconview workspace
MODULES_ENT	= network canvas table xml opengl sql
MODULES		= $$MODULES_BASE $$MODULES_PRO
enterprise:MODULES	+= $$MODULES_ENT

internal:MODULES	+= $$MODULES_ENT

internal:CONFIG		+= $$MODULES

internal:CONFIG	+= png zlib  # Done differently in external system
embedded:CONFIG	+= png zlib
win32:CONFIG	+= png zlib
internal:CONFIG -= opengl
# internal:LIBS += -lpng -lz

thread:TARGET = qt-mt
thread:DEFINES += QT_THREAD_SUPPORT

#just to be safe..
embedded:CONFIG -= opengl

# Use line like this for configs specific to your work
#

# CONFIG += opengl
#internal:CONFIG += mng
!mng:DEFINES += QT_NO_IMAGEIO_MNG

internal:CONFIG +=nas
nas:DEFINES     += QT_NAS_SUPPORT
nas:LIBS	+= -laudio -lXt

internal:CONFIG+= x11sm

# Install jpegsrc.v6b.tar.gz (find with http://ftpsearch.lycos.com)
#
internal:CONFIG += jpeg
embedded:CONFIG -= jpeg
win32:CONFIG -= jpeg
jpeg:INTJPEGIU += -ljpeg
jpeg:INTJPEGIW += $(QTDIR)/lib/libjpeg.lib
unix:INTJPEGI += $$INTJPEGIU
win32:INTJPEGI += $$INTJPEGIW
jpeg:LIBS += $$INTJPEGI
######
!jpeg:DEFINES += QT_NO_IMAGEIO_JPEG
win32:DEFINES += QT_NO_IMAGEIO_JPEG

#use Qt gif
gif:DEFINES += QT_BUILTIN_GIF_READER

#sm
!x11sm:DEFINES += QT_NO_SM_SUPPORT

win32:LIBS += $$WINLIBS
unix:LIBS += $$X11LIBS -ldl
embedded:LIBS -= $$X11LIBS

# next few lines add cups support
cups:DEFINES += QT_CUPS_SUPPORT
cups:LIBS += -lcups

mng:LIBS	+= -lmng -ljpeg
#mng:LIBS	+= -L$(QTDIR)/src/3rdparty/libmng -lmng -ljpeg

#DEFINES	+= QT_NO_ASCII_CAST
#DEFINES	+= QT_NO_CAST_ASCII

unix:CONFIG	       += x11inc
unix:TMAKE_CXXFLAGS    += -DQT_FATAL_ASSERT

win32:TMAKE_CFLAGS     += -DUNICODE
win32:TMAKE_CXXFLAGS   += -DUNICODE
#win32:TMAKE_CFLAGS    += -MT
#win32:TMAKE_CXXFLAGS  += -MT

MNG_INCLUDEPATH		= 3rdparty/libmng
PNG_INCLUDEPATH		= 3rdparty/libpng
ZLIB_INCLUDEPATH	= 3rdparty/zlib
#mng:INCLUDEPATH        += $$MNG_INCLUDEPATH
png:INCLUDEPATH        += $$PNG_INCLUDEPATH
zlib:INCLUDEPATH       += $$ZLIB_INCLUDEPATH
win32:INCLUDEPATH      += tmp
win32-borland:INCLUDEPATH += kernel


ps2:GSOS_INCLUDEPATH        = 3rdparty/gsos
ps2:INCLUDEPATH         += $$GSOS_INCLUDEPATH
ps2:QWSSUBLIBS += gsos
ps2:MAKELIBgsos = $(MAKE) -C 3rdparty/gsos; \
			cp 3rdparty/gsos/libgsos.a tmp
!ps2:DEFINES += QT_NO_QWS_PS2

win32:MOC_DIR	  = tmp
win32:OBJECTS_DIR = tmp

win32:DIALOGS_H	= ../include
win32:KERNEL_H	= ../include
win32:TOOLS_H	= ../include
win32:WIDGETS_H	= ../include
win32:OPENGL_H	= ../include
win32:NETWORK_H	= ../include
win32:CANVAS_H	= ../include
win32:TABLE_H	= ../include
win32:ICONVIEW_H	= ../include
win32:XML_H	= ../include
win32:WORKSPACE_H	= ../include
win32:SQL_H	= ../include

unix:DIALOGS_H	= dialogs
unix:KERNEL_H	= kernel
unix:TOOLS_H	= tools
unix:WIDGETS_H	= widgets
unix:OPENGL_H	= opengl
unix:NETWORK_H	= network
unix:CANVAS_H	= canvas
unix:TABLE_H	= table
unix:ICONVIEW_H	= iconview
unix:XML_H	= xml
unix:WORKSPACE_H	= workspace
unix:SQL_H	= sql

DIALOGS_P	= dialogs
KERNEL_P	= kernel
TOOLS_P		= tools
WIDGETS_P	= widgets

win32:DEPENDPATH = ../include
unix:DEPENDPATH	= $$DIALOGS_H:$$KERNEL_H:$$TOOLS_H:$$WIDGETS_H:$$OPENGL_H:$$NETWORK_H:$$CANVAS_H:$$TABLE_H:$$ICONVIEW_H:$$XML_H:$$WORKSPACE_H:$$SQL_H

dialogs:HEADERS	+= $$DIALOGS_H/qcolordialog.h \
		  $$DIALOGS_H/qerrormessage.h \
		  $$DIALOGS_H/qfiledialog.h \
		  $$DIALOGS_H/qfontdialog.h \
		  $$DIALOGS_H/qmessagebox.h \
		  $$DIALOGS_H/qprogressdialog.h \
		  $$DIALOGS_H/qtabdialog.h \
		  $$DIALOGS_H/qwizard.h \
		  $$DIALOGS_H/qinputdialog.h

kernel:HEADERS += $$KERNEL_H/qabstractlayout.h \
		  $$KERNEL_H/qaccel.h \
		  $$KERNEL_H/qapplication.h \
		  $$KERNEL_H/qapplication_p.h \
		  $$KERNEL_H/qasyncimageio.h \
		  $$KERNEL_H/qasyncio.h \
		  $$KERNEL_H/qbitmap.h \
		  $$KERNEL_H/qbrush.h \
		  $$KERNEL_H/qcleanuphandler.h \
		  $$KERNEL_H/qclipboard.h \
		  $$KERNEL_H/qcolor.h \
		  $$KERNEL_H/qcolor_p.h \
		  $$KERNEL_H/qcomponentinterface.h \
		  $$KERNEL_H/qconnection.h \
		  $$KERNEL_H/qcursor.h \
		  $$KERNEL_H/qdialog.h \
		  $$KERNEL_H/qdragobject.h \
		  $$KERNEL_H/qdrawutil.h \
		  $$KERNEL_H/qdropsite.h \
		  $$KERNEL_H/qevent.h \
		  $$KERNEL_H/qfocusdata.h \
		  $$KERNEL_H/qfont.h \
		  $$KERNEL_P/qfontdata_p.h \
		  $$KERNEL_H/qfontinfo.h \
		  $$KERNEL_H/qfontmetrics.h \
		  $$KERNEL_H/qguardedptr.h \
		  $$KERNEL_H/qgif.h \
		  $$KERNEL_H/qiconset.h \
		  $$KERNEL_H/qimage.h \
		  $$KERNEL_H/qkeycode.h \
		  $$KERNEL_H/qjpegio.h \
		  $$KERNEL_H/qlayout.h \
		  $$KERNEL_P/qlayoutengine_p.h \
		  $$KERNEL_H/qtranslator.h \
		  $$KERNEL_H/qmetaobject.h \
		  $$KERNEL_H/qmime.h \
		  $$KERNEL_H/qmngio.h \
		  $$KERNEL_H/qmovie.h \
		  $$KERNEL_H/qnamespace.h \
		  $$KERNEL_H/qnetworkprotocol.h \
		  $$KERNEL_H/qobject.h \
		  $$KERNEL_H/qobjectdefs.h \
		  $$KERNEL_H/qobjectdict.h \
		  $$KERNEL_H/qobjectlist.h \
		  $$KERNEL_H/qpaintdevice.h \
		  $$KERNEL_H/qpaintdevicedefs.h \
		  $$KERNEL_H/qpainter.h \
		  $$KERNEL_H/qpainter_p.h \
		  $$KERNEL_H/qpalette.h \
		  $$KERNEL_H/qpaintdevicemetrics.h \
		  $$KERNEL_H/qpen.h \
		  $$KERNEL_H/qpicture.h \
		  $$KERNEL_H/qpixmap.h \
		  $$KERNEL_H/qpixmapcache.h \
		  $$KERNEL_H/qplugin.h \
		  $$KERNEL_H/qinterfacemanager.h \
		  $$KERNEL_H/qpngio.h \
		  $$KERNEL_H/qpointarray.h \
		  $$KERNEL_H/qpoint.h \
		  $$KERNEL_H/qpolygonscanner.h \
		  $$KERNEL_H/qprinter.h \
		  $$KERNEL_H/qprocess.h \
		  $$KERNEL_H/qrect.h \
		  $$KERNEL_H/qregion.h \
		  $$KERNEL_H/qsemimodal.h \
		  $$KERNEL_H/qsessionmanager.h \
		  $$KERNEL_H/qsignal.h \
		  $$KERNEL_H/qsignalmapper.h \
		  $$KERNEL_H/qsignalslotimp.h \
		  $$KERNEL_H/qsize.h \
		  $$KERNEL_H/qsizegrip.h \
		  $$KERNEL_H/qsizepolicy.h \
		  $$KERNEL_H/qsocketnotifier.h \
		  $$KERNEL_H/qsound.h \
		  $$KERNEL_H/qstyle.h \
		  $$KERNEL_H/qstylesheet.h \
		  $$KERNEL_H/qthread.h \
		  $$KERNEL_H/qthread_p.h \
		  $$KERNEL_H/qtimer.h \
		  $$KERNEL_H/qurl.h \
		  $$KERNEL_H/qlocalfs.h \
		  $$KERNEL_H/qurloperator.h \
		  $$KERNEL_H/qurlinfo.h \
		  $$KERNEL_H/qwidget.h \
		  $$KERNEL_H/qwidgetintdict.h \
		  $$KERNEL_H/qwidgetlist.h \
		  $$KERNEL_H/qwindowdefs.h \
		  $$KERNEL_H/qwmatrix.h \
		  $$KERNEL_H/qvariant.h



tools:HEADERS +=  $$TOOLS_H/qarray.h \
		  $$TOOLS_H/qasciicache.h \
		  $$TOOLS_H/qasciidict.h \
		  $$TOOLS_H/qbig5codec.h \
		  $$TOOLS_H/qbitarray.h \
		  $$TOOLS_H/qbuffer.h \
		  $$TOOLS_H/qcache.h \
		  $$TOOLS_H/qcollection.h \
		  $$TOOLS_H/qcstring.h \
		  $$TOOLS_H/qdatastream.h \
		  $$TOOLS_H/qdatetime.h \
		  $$TOOLS_H/qdict.h \
		  $$TOOLS_H/qdir.h \
		  $$TOOLS_H/qeucjpcodec.h \
		  $$TOOLS_H/qeuckrcodec.h \
		  $$TOOLS_H/qfile.h \
		  $$TOOLS_P/qfiledefs_p.h \
		  $$TOOLS_H/qfileinfo.h \
		  $$TOOLS_H/qgarray.h \
		  $$TOOLS_H/qgbkcodec.h \
		  $$TOOLS_H/qgcache.h \
		  $$TOOLS_H/qgdict.h \
		  $$TOOLS_H/qgeneric.h \
		  $$TOOLS_H/qglist.h \
		  $$TOOLS_H/qglobal.h \
		  $$TOOLS_H/qgvector.h \
		  $$TOOLS_H/qintcache.h \
		  $$TOOLS_H/qintdict.h \
		  $$TOOLS_H/qiodevice.h \
		  $$TOOLS_H/qjiscodec.h \
		  $$TOOLS_H/qjpunicode.h \
		  $$TOOLS_H/qlist.h \
		  $$TOOLS_H/qmap.h \
		  $$TOOLS_H/qptrdict.h \
		  $$TOOLS_H/qqueue.h \
		  $$TOOLS_H/qregexp.h \
		  $$TOOLS_H/qrtlcodec.h \
		  $$TOOLS_H/qshared.h \
		  $$TOOLS_H/qsjiscodec.h \
		  $$TOOLS_H/qsortedlist.h \
		  $$TOOLS_H/qstack.h \
		  $$TOOLS_H/qstring.h \
		  $$TOOLS_H/qstringlist.h \
		  $$TOOLS_H/qstrlist.h \
		  $$TOOLS_H/qstrvec.h \
		  $$TOOLS_H/qtextcodec.h \
		  $$TOOLS_H/qtextstream.h \
		  $$TOOLS_H/qtsciicodec.h \
		  $$TOOLS_H/qutfcodec.h \
		  $$TOOLS_H/qvector.h \
	          $$TOOLS_H/qvaluelist.h

widgets:HEADERS += $$WIDGETS_H/qbuttongroup.h \
		  $$WIDGETS_H/qbutton.h \
		  $$WIDGETS_H/qcheckbox.h \
		  $$WIDGETS_H/qcdestyle.h \
		  $$WIDGETS_H/qcombobox.h \
		  $$WIDGETS_H/qcommonstyle.h \
		  $$WIDGETS_H/qwidgetresizehandler.h \
		  $$WIDGETS_H/qdial.h \
		  $$WIDGETS_H/qdockarea.h \
		  $$WIDGETS_H/qdockwindow.h \
		  $$WIDGETS_H/qframe.h \
		  $$WIDGETS_H/qgrid.h \
		  $$WIDGETS_H/qgroupbox.h \
		  $$WIDGETS_H/qhbuttongroup.h \
		  $$WIDGETS_H/qheader.h \
		  $$WIDGETS_H/qhgroupbox.h \
		  $$WIDGETS_H/qhbox.h \
		  $$WIDGETS_H/qlabel.h \
		  $$WIDGETS_H/qlcdnumber.h \
		  $$WIDGETS_H/qlineedit.h \
		  $$WIDGETS_H/qlistbox.h \
		  $$WIDGETS_H/qlistview.h \
		  $$WIDGETS_H/qmainwindow.h \
		  $$WIDGETS_H/qmenubar.h \
		  $$WIDGETS_H/qmenudata.h \
		  $$WIDGETS_H/qmotifstyle.h \
		  $$WIDGETS_H/qmotifplusstyle.h \
		  $$WIDGETS_H/qmultilineedit.h \
		  $$WIDGETS_H/qplatinumstyle.h \
		  $$WIDGETS_H/qpopupmenu.h \
		  $$WIDGETS_H/qprogressbar.h \
		  $$WIDGETS_H/qpushbutton.h \
		  $$WIDGETS_H/qradiobutton.h \
		  $$WIDGETS_H/qrangecontrol.h \
		  $$WIDGETS_H/qscrollbar.h \
		  $$WIDGETS_H/qscrollview.h \
		  $$WIDGETS_H/qsgistyle.h \
		  $$WIDGETS_H/qslider.h \
		  $$WIDGETS_H/qspinbox.h \
		  $$WIDGETS_H/qsplitter.h \
		  $$WIDGETS_H/qstatusbar.h \
		  $$WIDGETS_H/qtabbar.h \
		  $$WIDGETS_H/qtabwidget.h \
		  $$WIDGETS_H/qtableview.h \
		  $$WIDGETS_H/qtoolbar.h \
		  $$WIDGETS_H/qtoolbutton.h \
		  $$WIDGETS_H/qtooltip.h \
		  $$WIDGETS_H/qvalidator.h \
		  $$WIDGETS_H/qvbox.h \
		  $$WIDGETS_H/qvbuttongroup.h \
		  $$WIDGETS_H/qvgroupbox.h \
		  $$WIDGETS_H/qwhatsthis.h \
		  $$WIDGETS_H/qwidgetstack.h \
		  $$WIDGETS_H/qwidgetresizehandler.h \
		  $$WIDGETS_H/qwindowsstyle.h \
		  $$WIDGETS_H/qaction.h

tools:WINSOURCES += tools/qdir_win.cpp \
	 	  tools/qfile_win.cpp \
		  tools/qfileinfo_win.cpp

kernel:WINSOURCES += kernel/qapplication_win.cpp \
		  kernel/qclipboard_win.cpp \
		  kernel/qcolor_win.cpp \
		  kernel/qcursor_win.cpp \
		  kernel/qdnd_win.cpp \
		  kernel/qfont_win.cpp \
		  kernel/qmime_win.cpp \
		  kernel/qpixmap_win.cpp \
		  kernel/qprinter_win.cpp \
		  kernel/qprocess_win.cpp \
		  kernel/qpaintdevice_win.cpp \
		  kernel/qpainter_win.cpp \
		  kernel/qregion_win.cpp \
		  kernel/qsound_win.cpp \
		  kernel/qthread_win.cpp \
		  kernel/qwidget_win.cpp \
		  kernel/qole_win.c

dialogs:WINSOURCES += dialogs/qfiledialog_win.cpp

win32:SOURCES += $$WINSOURCES

tools:UNIXSOURCES += tools/qdir_unix.cpp \
		  tools/qfile_unix.cpp \
		  tools/qfileinfo_unix.cpp

kernel:QWSSOURCES += kernel/qapplication_qws.cpp \
		  kernel/qclipboard_qws.cpp \
		  kernel/qcolor_qws.cpp \
		  kernel/qcursor_qws.cpp \
		  kernel/qdnd_qws.cpp \
		  kernel/qfont_qws.cpp \
		  kernel/qpixmap_qws.cpp \
		  kernel/qprinter_qws.cpp \
		  kernel/qpaintdevice_qws.cpp \
		  kernel/qpainter_qws.cpp \
		  kernel/qregion_qws.cpp \
		  kernel/qsound_qws.cpp \
		  kernel/qwidget_qws.cpp \
		  kernel/qgfx_qws.cpp \
		  kernel/qgfxraster_qws.cpp \
		  kernel/qfontmanager_qws.cpp \
		  kernel/qfontfactorybdf_qws.cpp \
		  kernel/qfontfactoryttf_qws.cpp \
		  kernel/qmemorymanager_qws.cpp \
		  kernel/qwscommand_qws.cpp \
		  kernel/qwsevent_qws.cpp \
		  kernel/qwindowsystem_qws.cpp \
		  kernel/qkeyboard_qws.cpp \
		  kernel/qwscursor_qws.cpp \
		  kernel/qwsmouse_qws.cpp \
		  kernel/qwsmanager_qws.cpp \
		  kernel/qwsproperty_qws.cpp \
		  kernel/qlock_qws.cpp \
		  kernel/qwsregionmanager_qws.cpp \
		  kernel/qwssocket_qws.cpp

oldx11font:X11FONTSOURCES = kernel/qfont_x11.cpp
newx11font:X11FONTSOURCES = ../tests/newfont/qfont_newx11.cpp

kernel:X11SOURCES += kernel/qapplication_x11.cpp \
		  kernel/qclipboard_x11.cpp \
		  kernel/qcolor_x11.cpp \
		  kernel/qcursor_x11.cpp \
		  kernel/qdnd_x11.cpp \
		  kernel/qmotifdnd_x11.cpp \
		  kernel/qpixmap_x11.cpp \
		  kernel/qprinter_x11.cpp \
		  kernel/qpaintdevice_x11.cpp \
		  kernel/qpainter_x11.cpp \
		  kernel/qregion_x11.cpp \
		  kernel/qsound_x11.cpp \
		  kernel/qwidget_x11.cpp \
		  kernel/qnpsupport.cpp \
		  kernel/qwidgetcreate_x11.cpp \
		  $$X11FONTSOURCES

widgets:QWSSOURCES += $$WIDGETS_H/qcompactstyle.cpp

kernel:UNIXSOURCES += kernel/qpsprinter.cpp \
		    kernel/qprocess_unix.cpp \
		    kernel/qthread_unix.cpp

dialogs:UNIXSOURCES += dialogs/qprintdialog.cpp

internal:X11SOURCES += $$WIDGETS_H/qcompactstyle.cpp
unix:SOURCES += $$UNIXSOURCES
unix:SOURCES += $$X11SOURCES
embedded:SOURCES -= $$X11SOURCES
embedded:SOURCES += $$QWSSOURCES

tools:SOURCES += tools/qbig5codec.cpp \
		  tools/qbitarray.cpp \
		  tools/qbuffer.cpp \
		  tools/qcollection.cpp \
		  tools/qcstring.cpp \
		  tools/qdatastream.cpp \
		  tools/qdatetime.cpp \
		  tools/qdir.cpp \
		  tools/qeucjpcodec.cpp \
		  tools/qeuckrcodec.cpp \
		  tools/qfile.cpp \
		  tools/qfileinfo.cpp \
		  tools/qgarray.cpp \
		  tools/qgbkcodec.cpp \
		  tools/qgcache.cpp \
		  tools/qgdict.cpp \
		  tools/qglist.cpp \
		  tools/qglobal.cpp \
		  tools/qgvector.cpp \
		  tools/qiodevice.cpp \
		  tools/qjiscodec.cpp \
		  tools/qjpunicode.cpp \
		  tools/qmap.cpp \
		  tools/qregexp.cpp \
		  tools/qrtlcodec.cpp \
		  tools/qsjiscodec.cpp \
		  tools/qstring.cpp \
		  tools/qstringlist.cpp \
		  tools/qtextcodec.cpp \
		  tools/qtextstream.cpp \
		  tools/qtsciicodec.cpp \
		  tools/qutfcodec.cpp

kernel:SOURCES += kernel/qabstractlayout.cpp \
		  kernel/qaccel.cpp \
		  kernel/qapplication.cpp \
		  kernel/qasyncimageio.cpp \
		  kernel/qasyncio.cpp \
		  kernel/qbitmap.cpp \
		  kernel/qclipboard.cpp \
		  kernel/qcolor.cpp \
		  kernel/qcolor_p.cpp \
		  kernel/qcomponentinterface.cpp \
		  kernel/qconnection.cpp \
		  kernel/qcursor.cpp \
		  kernel/qdialog.cpp \
		  kernel/qdragobject.cpp \
		  kernel/qdrawutil.cpp \
		  kernel/qdropsite.cpp \
		  kernel/qevent.cpp \
		  kernel/qfocusdata.cpp \
		  kernel/qfont.cpp \
		  kernel/qfontdatabase.cpp \
		  kernel/qguardedptr.cpp \
		  kernel/qiconset.cpp \
		  kernel/qimage.cpp \
		  kernel/qjpegio.cpp \
		  kernel/qlayout.cpp \
		  kernel/qlayoutengine.cpp \
		  kernel/qtranslator.cpp \
		  kernel/qmetaobject.cpp \
		  kernel/qmime.cpp \
		  kernel/qmngio.cpp \
		  kernel/qmovie.cpp \
		  kernel/qnetworkprotocol.cpp \
		  kernel/qobject.cpp \
		  kernel/qpainter.cpp \
		  kernel/qpalette.cpp \
		  kernel/qpaintdevicemetrics.cpp \
		  kernel/qpicture.cpp \
		  kernel/qpixmap.cpp \
		  kernel/qpixmapcache.cpp \
		  kernel/qplugin.cpp \
		  kernel/qpngio.cpp \
		  kernel/qpointarray.cpp \
		  kernel/qpoint.cpp \
		  kernel/qpolygonscanner.cpp \
		  kernel/qprinter.cpp \
		  kernel/qprocess.cpp \
		  kernel/qrect.cpp \
		  kernel/qregion.cpp \
		  kernel/qsignal.cpp \
		  kernel/qsignalmapper.cpp \
		  kernel/qsize.cpp \
		  kernel/qsizegrip.cpp \
		  kernel/qstyle.cpp \
		  kernel/qsocketnotifier.cpp \
		  kernel/qsound.cpp \
		  kernel/qstylesheet.cpp \
		  kernel/qtimer.cpp \
		  kernel/qurl.cpp \
		  kernel/qlocalfs.cpp \
		  kernel/qurloperator.cpp \
		  kernel/qurlinfo.cpp \
		  kernel/qwidget.cpp \
		  kernel/qwmatrix.cpp \
		  kernel/qvariant.cpp

widgets:SOURCES += widgets/qbuttongroup.cpp \
		  widgets/qbutton.cpp \
		  widgets/qcdestyle.cpp \
		  widgets/qcheckbox.cpp \
		  widgets/qcombobox.cpp \
		  widgets/qwidgetresizehandler.cpp \
		  widgets/qcommonstyle.cpp \
		  widgets/qdial.cpp \
		  widgets/qdockarea.cpp \
		  widgets/qdockwindow.cpp \
		  widgets/qframe.cpp \
		  widgets/qgrid.cpp \
		  widgets/qgroupbox.cpp \
		  widgets/qhbuttongroup.cpp \
		  widgets/qheader.cpp \
		  widgets/qhgroupbox.cpp \
		  widgets/qhbox.cpp \
		  widgets/qlabel.cpp \
		  widgets/qlcdnumber.cpp \
		  widgets/qlineedit.cpp \
		  widgets/qlistbox.cpp \
		  widgets/qlistview.cpp \
		  widgets/qmainwindow.cpp \
		  widgets/qmenubar.cpp \
		  widgets/qmenudata.cpp \
		  widgets/qmotifstyle.cpp \
		  widgets/qmotifplusstyle.cpp \
		  widgets/qmultilineedit.cpp \
		  widgets/qplatinumstyle.cpp \
		  widgets/qpopupmenu.cpp \
		  widgets/qprogressbar.cpp \
		  widgets/qpushbutton.cpp \
		  widgets/qradiobutton.cpp \
		  widgets/qrangecontrol.cpp \
		  widgets/qscrollbar.cpp \
		  widgets/qscrollview.cpp \
		  widgets/qsgistyle.cpp \
		  widgets/qslider.cpp \
		  widgets/qspinbox.cpp \
		  widgets/qsplitter.cpp \
		  widgets/qstatusbar.cpp \
		  widgets/qtabbar.cpp \
		  widgets/qtabwidget.cpp \
		  widgets/qtableview.cpp \
		  widgets/qtoolbar.cpp \
		  widgets/qtoolbutton.cpp \
		  widgets/qtooltip.cpp \
		  widgets/qvalidator.cpp \
		  widgets/qvbox.cpp \
		  widgets/qvbuttongroup.cpp \
		  widgets/qvgroupbox.cpp \
		  widgets/qwhatsthis.cpp \
		  widgets/qwidgetresizehandler.cpp \
		  widgets/qwidgetstack.cpp \
		  widgets/qwindowsstyle.cpp \
		  widgets/qaction.cpp \
		  widgets/qeffects.cpp

dialogs:SOURCES += dialogs/qcolordialog.cpp \
		  dialogs/qerrormessage.cpp \
		  dialogs/qfiledialog.cpp \
		  dialogs/qfontdialog.cpp \
		  dialogs/qmessagebox.cpp \
		  dialogs/qprogressdialog.cpp \
		  dialogs/qtabdialog.cpp \
		  dialogs/qwizard.cpp \
		  dialogs/qinputdialog.cpp

unix:HEADERS   += $$DIALOGS_H/qprintdialog.h \
		  $$KERNEL_P/qpsprinter_p.h \
		  $$KERNEL_H/qfontdatabase.h

unix:HEADERS   += $$WIDGETS_H/qcompactstyle.h
embedded:HEADERS   -= $$WIDGETS_H/qcompactstyle.h

embedded:HEADERS += $$KERNEL_H/qfontmanager_qws.h \
		  $$KERNEL_H/qfontfactorybdf_qws.h \
		  $$KERNEL_H/qfontfactoryttf_qws.h \
		  $$KERNEL_H/qmemorymanager_qws.h \
		  $$KERNEL_H/qwsmanager_qws.h \
		  $$KERNEL_H/qgfx_qws.h \
		  $$KERNEL_H/qgfxraster_qws.h \
		  $$KERNEL_H/qgfxlinuxfb_qws.h \
		  $$KERNEL_H/qgfxvnc_qws.h \
		  $$KERNEL_H/qwindowsystem_qws.h \
		  $$KERNEL_H/qwscursor_qws.h \
		  $$KERNEL_H/qwsmouse_qws.h \
		  $$KERNEL_H/qlock_qws.h \
		  $$KERNEL_H/qwsregionmanager_qws.h \
		  $$KERNEL_H/qwsdisplay_qws.h \
		  $$KERNEL_H/qwssocket_qws.h \
		  $$WIDGETS_H/qcompactstyle.h

PNG_SOURCES	= 3rdparty/libpng/png.c \
		  3rdparty/libpng/pngerror.c \
		  3rdparty/libpng/pngget.c \
		  3rdparty/libpng/pngmem.c \
		  3rdparty/libpng/pngpread.c \
		  3rdparty/libpng/pngread.c \
		  3rdparty/libpng/pngrio.c \
		  3rdparty/libpng/pngrtran.c \
		  3rdparty/libpng/pngrutil.c \
		  3rdparty/libpng/pngset.c \
		  3rdparty/libpng/pngtrans.c \
		  3rdparty/libpng/pngwio.c \
		  3rdparty/libpng/pngwrite.c \
		  3rdparty/libpng/pngwtran.c \
		  3rdparty/libpng/pngwutil.c

ZLIB_SOURCES	= 3rdparty/zlib/adler32.c \
		  3rdparty/zlib/compress.c \
		  3rdparty/zlib/crc32.c \
		  3rdparty/zlib/deflate.c \
		  3rdparty/zlib/gzio.c \
		  3rdparty/zlib/infblock.c \
		  3rdparty/zlib/infcodes.c \
		  3rdparty/zlib/inffast.c \
		  3rdparty/zlib/inflate.c \
		  3rdparty/zlib/inftrees.c \
		  3rdparty/zlib/infutil.c \
		  3rdparty/zlib/trees.c \
		  3rdparty/zlib/uncompr.c \
		  3rdparty/zlib/zutil.c

png:SOURCES    += $$PNG_SOURCES
embedded:SOURCES    -= $$PNG_SOURCES
zlib:SOURCES   += $$ZLIB_SOURCES
embedded:SOURCES    -= $$ZLIB_SOURCES


xml:HEADERS += $$XML_H/qxml.h $$XML_H/qdom.h
xml:SOURCES += xml/qxml.cpp xml/qdom.cpp

workspace:HEADERS += $$WORKSPACE_H/qworkspace.h
workspace:SOURCES += workspace/qworkspace.cpp

canvas:HEADERS += $$CANVAS_H/qcanvas.h
canvas:SOURCES += canvas/qcanvas.cpp

iconview:HEADERS += $$ICONVIEW_H/qiconview.h
iconview:SOURCES += iconview/qiconview.cpp

table:HEADERS += $$TABLE_H/qtable.h
table:SOURCES += table/qtable.cpp

!sql:DEFINES += QT_NO_SQL
sql:HEADERS += $$SQL_H/qsql.h \
		    $$SQL_H/qsqlconnection.h \
		    $$SQL_H/qsqldatabase.h \
		    $$SQL_H/qsqlfield.h \
		    $$SQL_H/qsqlview.h \
		    $$SQL_H/qsqlform.h \
		    $$SQL_H/qsqleditorfactory.h \
		    $$SQL_H/qsqldriver.h \
		    $$SQL_H/qsqldriverinterface.h \
		    $$SQL_H/qsqldriverplugin.h \
		    $$SQL_H/qsqlerror.h \
		    $$SQL_H/qsqlresult.h \
		    $$SQL_H/qsqlindex.h \
		    $$SQL_H/qsqltable.h
sql:SOURCES += sql/qsql.cpp \
		    sql/qsqldatabase.cpp \
		    sql/qsqlconnection.cpp \
		    sql/qsqlfield.cpp \
		    sql/qsqlform.cpp \
		    sql/qsqlview.cpp \
		    sql/qsqleditorfactory.cpp \
		    sql/qsqldriver.cpp \
		    sql/qsqldriverplugin.cpp \
		    sql/qsqlerror.cpp \
		    sql/qsqlresult.cpp \
		    sql/qsqlindex.cpp \
		    sql/qsqltable.cpp

sql_postgres:HEADERS += $$SQL_H/src/psql/qsql_psql.h
sql_postgres:SOURCES += sql/src/psql/qsql_psql.cpp
sql_postgres:DEFINES += QT_SQL_POSTGRES
unix:sql_postgres:INCLUDEPATH += /usr/include/postgresql
unix:sql_postgres:LIBS += -lpq

sql_mysql:HEADERS += $$SQL_H/src/mysql/qsql_mysql.h
sql_mysql:SOURCES += sql/src/mysql/qsql_mysql.cpp
sql_mysql:DEFINES += QT_SQL_MYSQL
unix:sql_mysql:INCLUDEPATH += /usr/include/mysql
unix:sql_mysql:LIBS += -lmysqlclient

sql_odbc:HEADERS += $$SQL_H/src/odbc/qsql_odbc.h
sql_odbc:SOURCES += sql/src/odbc/qsql_odbc.cpp
sql_odbc:DEFINES += QT_SQL_ODBC
unix:sql_odbc:INCLUDEPATH += /usr/local/include
unix:sql_odbc:LIBS += -lodbc

opengl:HEADERS += $$OPENGL_H/qgl.h
OPENGL_SOURCES	= opengl/qgl.cpp
unix:OPENGL_SOURCES += opengl/qgl_x11.cpp
win32:OPENGL_SOURCES += opengl/qgl_win.cpp
opengl:SOURCES    += $$OPENGL_SOURCES

network:HEADERS += $$NETWORK_H/qdns.h \
		    $$NETWORK_H/qftp.h \
		    $$NETWORK_H/qhttp.h \
		    $$NETWORK_H/qhostaddress.h \
		    $$NETWORK_H/qnetwork.h \
		    $$NETWORK_H/qserversocket.h \
		    $$NETWORK_H/qsocket.h \
		    $$NETWORK_H/qsocketdevice.h
NETWORK_SOURCES	= network/qdns.cpp \
		    network/qftp.cpp \
		    network/qhttp.cpp \
		    network/qhostaddress.cpp \
		    network/qnetwork.cpp \
		    network/qserversocket.cpp \
		    network/qsocket.cpp \
		    network/qsocketdevice.cpp
unix:NETWORK_SOURCES += network/qsocketdevice_unix.cpp
win32:NETWORK_SOURCES += network/qsocketdevice_win.cpp
network:SOURCES    += $$NETWORK_SOURCES

oldrichtext:SOURCES += kernel/qrichtext.cpp \
			kernel/qsimplerichtext.cpp \
			widgets/qtextview.cpp \
			widgets/qtextbrowser.cpp
newrichtext:SOURCES += ../tests/qtextedit/qrichtext.cpp \
			../tests/qtextedit/qsimplerichtext.cpp \
			../tests/qtextedit/qtextedit.cpp \
			../tests/qtextedit/qtextbrowser.cpp \
			../tests/qtextedit/qtextview.cpp
oldrichtext:HEADERS += kernel/qrichtext_p.h \
			kernel/qsimplerichtext.h \
			widgets/qtextview.h \
			widgets/qtextbrowser.h
newrichtext:HEADERS += ../tests/qtextedit/qrichtext_p.h \
			../tests/qtextedit/qsimplerichtext.h \
			../tests/qtextedit/qtextedit.h \
			../tests/qtextedit/qtextbrowser.h \
			../tests/qtextedit/qtextview.h
newrichtext:DEFINES += QT_NEW_RICHTEXT

# Qt/Embedded
embedded:PRECOMPH=$(QTDIR)/src/kernel/qt.h
embedded:INCLUDEPATH += 3rdparty/freetype2/include 3rdparty/libpng 3rdparty/zlib
QWSSUBLIBS += freetype
png:QWSSUBLIBS += png
mng:QWSSUBLIBS += mng
zlib:QWSSUBLIBS += z
jpeg:QWSSUBLIBS += jpeg
embedded:SUBLIBS = $$QWSSUBLIBS
embedded:MAKELIBz = $(MAKE) -C 3rdparty/zlib -f Makefile$$DASHCROSS; \
			cp 3rdparty/zlib/libz.a tmp
embedded:MAKELIBfreetype = $(MAKE) -C 3rdparty/freetype2 CONFIG_MK=config$$DASHCROSS.mk OBJ_DIR=../../tmp \
			    ../../tmp/libfreetype.a
embedded:MAKELIBpng = $(MAKE) -C 3rdparty/libpng \
			    -f scripts/makefile.linux$$DASHCROSS; \
			    cp 3rdparty/libpng/libpng.a tmp
embedded:MAKELIBmng = $(MAKE) -C 3rdparty/libmng \
			    -f makefiles/makefile.linux$$DASHCROSS; \
			    cp 3rdparty/libmng/libmng.a tmp
embedded:MAKELIBjpeg = $(MAKE) -C 3rdparty/jpeglib -f makefile.unix$$DASHCROSS; \
			    cp 3rdparty/jpeglib/libjpeg.a tmp


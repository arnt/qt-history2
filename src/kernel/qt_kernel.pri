# Qt kernel module

kernel {
	win32:KERNEL_H	= ../include
	unix:KERNEL_H	= kernel	
	KERNEL_P	= kernel
	unix:DEPENDPATH	+= :$$KERNEL_H
	HEADERS += $$KERNEL_H/qabstractlayout.h \
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

	win32:SOURCES += kernel/qapplication_win.cpp \
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
	
	embedded {
		SOURCES += kernel/qapplication_qws.cpp \
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
	}

	!embedded {
		unix {
			SOURCES += kernel/qapplication_x11.cpp \
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
			  kernel/qwidgetcreate_x11.cpp 
		}
	}

	unix:SOURCES += kernel/qpsprinter.cpp \
		    kernel/qprocess_unix.cpp \
		    kernel/qthread_unix.cpp

	SOURCES += kernel/qabstractlayout.cpp \
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

	unix:HEADERS   += $$KERNEL_P/qpsprinter_p.h \
			  $$KERNEL_H/qfontdatabase.h
}
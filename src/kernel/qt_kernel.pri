# Qt kernel module

kernel {
	KERNEL_P	= kernel
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
		  $$KERNEL_H/qlayout.h \
		  $$KERNEL_P/qlayoutengine_p.h \
		  $$KERNEL_H/qtranslator.h \
		  $$KERNEL_H/qmetaobject.h \
		  $$KERNEL_H/qmime.h \
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
		  $$KERNEL_H/qvariant.h \
		  kernel/qrichtext_p.h \
		  $$KERNEL_H/qsimplerichtext.h 


	win32:SOURCES += $$KERNEL_CPP/qapplication_win.cpp \
		  $$KERNEL_CPP/qclipboard_win.cpp \
		  $$KERNEL_CPP/qcolor_win.cpp \
		  $$KERNEL_CPP/qcursor_win.cpp \
		  $$KERNEL_CPP/qdnd_win.cpp \
		  $$KERNEL_CPP/qfont_win.cpp \
		  $$KERNEL_CPP/qmime_win.cpp \
		  $$KERNEL_CPP/qpixmap_win.cpp \
		  $$KERNEL_CPP/qprinter_win.cpp \
		  $$KERNEL_CPP/qprocess_win.cpp \
		  $$KERNEL_CPP/qpaintdevice_win.cpp \
		  $$KERNEL_CPP/qpainter_win.cpp \
		  $$KERNEL_CPP/qregion_win.cpp \
		  $$KERNEL_CPP/qsound_win.cpp \
		  $$KERNEL_CPP/qthread_win.cpp \
		  $$KERNEL_CPP/qwidget_win.cpp \
		  $$KERNEL_CPP/qole_win.c
	
	unix:x11 {
	      SOURCES += $$KERNEL_CPP/qapplication_x11.cpp \
		          $$KERNEL_CPP/qclipboard_x11.cpp \
			  $$KERNEL_CPP/qcolor_x11.cpp \
			  $$KERNEL_CPP/qcursor_x11.cpp \
			  $$KERNEL_CPP/qdnd_x11.cpp \
			  $$KERNEL_CPP/qmotifdnd_x11.cpp \
			  $$KERNEL_CPP/qpixmap_x11.cpp \
			  $$KERNEL_CPP/qprinter_x11.cpp \
			  $$KERNEL_CPP/qpaintdevice_x11.cpp \
			  $$KERNEL_CPP/qpainter_x11.cpp \
			  $$KERNEL_CPP/qregion_x11.cpp \
			  $$KERNEL_CPP/qsound_x11.cpp \
			  $$KERNEL_CPP/qwidget_x11.cpp \
			  $$KERNEL_CPP/qnpsupport.cpp \
			  $$KERNEL_CPP/qwidgetcreate_x11.cpp
	}

        macx {
	      SOURCES += $$KERNEL_CPP/qapplication_mac.cpp \
		          $$KERNEL_CPP/qclipboard_mac.cpp \
			  $$KERNEL_CPP/qcolor_mac.cpp \
			  $$KERNEL_CPP/qcursor_mac.cpp \
			  $$KERNEL_CPP/qdnd_mac.cpp \
			  $$KERNEL_CPP/qpixmap_mac.cpp \
			  $$KERNEL_CPP/qprinter_mac.cpp \
			  $$KERNEL_CPP/qpaintdevice_mac.cpp \
			  $$KERNEL_CPP/qpainter_mac.cpp \
			  $$KERNEL_CPP/qregion_mac.cpp \
			  $$KERNEL_CPP/qsound_mac.cpp \
			  $$KERNEL_CPP/qwidget_mac.cpp \
			  $$KERNEL_CPP/qnpsupport.cpp \
			  $$KERNEL_CPP/qfont_mac.cpp \
			  $$KERNEL_CPP/qwidgetcreate_mac.cpp
             DEFINES += QT_NO_PLUGIN ALLOW_OLD_CARBON _EVENT_HANDLERS=0 
             DEFINES += ALLOW_OLD_CREATE_FOO_CONTROL_PARAMETERS=0 CARBON_ON_MACH_O=1 
             DEFINES += ALLOW_OLD_BLOCKING_APIS=0 ALLOW_OLD_EVENT_LOOP_APIS=0
             LIBS += -framework Carbon
        }

	unix:SOURCES += $$KERNEL_CPP/qpsprinter.cpp \
		    $$KERNEL_CPP/qprocess_unix.cpp \
		    $$KERNEL_CPP/qthread_unix.cpp

	SOURCES += $$KERNEL_CPP/qabstractlayout.cpp \
		  $$KERNEL_CPP/qaccel.cpp \
		  $$KERNEL_CPP/qapplication.cpp \
		  $$KERNEL_CPP/qasyncimageio.cpp \
		  $$KERNEL_CPP/qasyncio.cpp \
		  $$KERNEL_CPP/qbitmap.cpp \
		  $$KERNEL_CPP/qclipboard.cpp \
		  $$KERNEL_CPP/qcolor.cpp \
		  $$KERNEL_CPP/qcolor_p.cpp \
		  $$KERNEL_CPP/qcomponentinterface.cpp \
		  $$KERNEL_CPP/qconnection.cpp \
		  $$KERNEL_CPP/qcursor.cpp \
		  $$KERNEL_CPP/qdialog.cpp \
		  $$KERNEL_CPP/qdragobject.cpp \
		  $$KERNEL_CPP/qdrawutil.cpp \
		  $$KERNEL_CPP/qdropsite.cpp \
		  $$KERNEL_CPP/qevent.cpp \
		  $$KERNEL_CPP/qfocusdata.cpp \
		  $$KERNEL_CPP/qfont.cpp \
		  $$KERNEL_CPP/qfontdatabase.cpp \
		  $$KERNEL_CPP/qguardedptr.cpp \
		  $$KERNEL_CPP/qiconset.cpp \
		  $$KERNEL_CPP/qimage.cpp \
		  $$KERNEL_CPP/qlayout.cpp \
		  $$KERNEL_CPP/qlayoutengine.cpp \
		  $$KERNEL_CPP/qtranslator.cpp \
		  $$KERNEL_CPP/qmetaobject.cpp \
		  $$KERNEL_CPP/qmime.cpp \
		  $$KERNEL_CPP/qmovie.cpp \
		  $$KERNEL_CPP/qnetworkprotocol.cpp \
		  $$KERNEL_CPP/qobject.cpp \
		  $$KERNEL_CPP/qpainter.cpp \
		  $$KERNEL_CPP/qpalette.cpp \
		  $$KERNEL_CPP/qpaintdevicemetrics.cpp \
		  $$KERNEL_CPP/qpicture.cpp \
		  $$KERNEL_CPP/qpixmap.cpp \
		  $$KERNEL_CPP/qpixmapcache.cpp \
		  $$KERNEL_CPP/qplugin.cpp \
		  $$KERNEL_CPP/qpointarray.cpp \
		  $$KERNEL_CPP/qpoint.cpp \
		  $$KERNEL_CPP/qpolygonscanner.cpp \
		  $$KERNEL_CPP/qprinter.cpp \
		  $$KERNEL_CPP/qprocess.cpp \
		  $$KERNEL_CPP/qrect.cpp \
		  $$KERNEL_CPP/qregion.cpp \
		  $$KERNEL_CPP/qsignal.cpp \
		  $$KERNEL_CPP/qsignalmapper.cpp \
		  $$KERNEL_CPP/qsize.cpp \
		  $$KERNEL_CPP/qsizegrip.cpp \
		  $$KERNEL_CPP/qstyle.cpp \
		  $$KERNEL_CPP/qsocketnotifier.cpp \
		  $$KERNEL_CPP/qsound.cpp \
		  $$KERNEL_CPP/qstylesheet.cpp \
		  $$KERNEL_CPP/qtimer.cpp \
		  $$KERNEL_CPP/qurl.cpp \
		  $$KERNEL_CPP/qlocalfs.cpp \
		  $$KERNEL_CPP/qurloperator.cpp \
		  $$KERNEL_CPP/qurlinfo.cpp \
		  $$KERNEL_CPP/qwidget.cpp \
		  $$KERNEL_CPP/qwmatrix.cpp \
		  $$KERNEL_CPP/qvariant.cpp \
		  $$KERNEL_CPP/qrichtext.cpp \
		  $$KERNEL_CPP/qsimplerichtext.cpp 

	unix:HEADERS   += $$KERNEL_P/qpsprinter_p.h \
			  $$KERNEL_H/qfontdatabase.h
}

# Qt kernel module

kernel {
	KERNEL_P	= kernel
	HEADERS += \
		  $$KERNEL_H/qpaintengine.h \
		  $$KERNEL_H/qabstractlayout.h \
		  $$KERNEL_H/qaccel.h \
		  $$KERNEL_H/qapplication.h \
		  $$KERNEL_H/qbitmap.h \
		  $$KERNEL_H/qbrush.h \
		  $$KERNEL_H/qclipboard.h \
		  $$KERNEL_H/qcolor.h \
		  $$KERNEL_H/qcursor.h \
		  $$KERNEL_H/qdesktopwidget.h \
		  $$KERNEL_H/qdragobject.h \
		  $$KERNEL_H/qdrawutil.h \
		  $$KERNEL_H/qdropsite.h \
		  $$KERNEL_H/qevent.h \
		  $$KERNEL_H/qfont.h \
		  $$KERNEL_H/qfontinfo.h \
		  $$KERNEL_H/qfontmetrics.h \
		  $$KERNEL_H/qgif.h \
		  $$KERNEL_H/qguieventloop.h\
		  $$KERNEL_H/qiconset.h \
		  $$KERNEL_H/qimage.h \
		  $$KERNEL_H/qimageformatplugin.h \
		  $$KERNEL_P/qimageformatinterface_p.h \
		  $$KERNEL_H/qkeycode.h \
		  $$KERNEL_H/qkeysequence.h \
		  $$KERNEL_H/qlayout.h \
		  $$KERNEL_H/qmime.h \
		  $$KERNEL_H/qmovie.h \
		  $$KERNEL_H/qpaintdevice.h \
		  $$KERNEL_H/qpaintdevicedefs.h \
		  $$KERNEL_H/qpaintdevicemetrics.h \
		  $$KERNEL_H/qpainter.h \
		  $$KERNEL_H/qpalette.h \
		  $$KERNEL_H/qpen.h \
		  $$KERNEL_H/qpicture.h \
		  $$KERNEL_P/qpictureformatinterface_p.h \
		  $$KERNEL_H/qpictureformatplugin.h \
		  $$KERNEL_H/qpixmap.h \
		  $$KERNEL_H/qpixmapcache.h \
		  $$KERNEL_H/qpoint.h \
		  $$KERNEL_H/qpointarray.h \
		  $$KERNEL_H/qpolygonscanner.h \
		  $$KERNEL_H/qprinter.h \
		  $$KERNEL_H/qrect.h \
		  $$KERNEL_H/qregion.h \
		  $$KERNEL_H/qsessionmanager.h \
		  $$KERNEL_H/qsize.h \
		  $$KERNEL_H/qsizegrip.h \
		  $$KERNEL_H/qsizepolicy.h \
		  $$KERNEL_H/qsound.h \
		  $$KERNEL_H/qstyle.h \
		  $$KERNEL_H/qstylesheet.h \
		  $$KERNEL_H/qwidget.h \
		  $$KERNEL_H/qwindowdefs.h \
		  $$KERNEL_H/qwmatrix.h \
		  $$KERNEL_P/qapplication_p.h \
	 	  $$KERNEL_H/qcolor_p.h \
		  $$KERNEL_P/qfontdata_p.h \
		  $$KERNEL_P/qguieventloop_p.h \
		  $$KERNEL_P/qlayoutengine_p.h \
		  $$KERNEL_P/qpainter_p.h \
	 	  $$KERNEL_P/qinputcontext_p.h \
		  $$KERNEL_P/qrichtext_p.h \
		  $$KERNEL_H/qsimplerichtext.h \
		  $$KERNEL_H/qscriptengine_p.h \
		  $$KERNEL_H/qtextengine_p.h \
		  $$KERNEL_H/qfontengine_p.h \
		  $$KERNEL_H/qtextlayout_p.h \
		  $$KERNEL_H/qtextformat.h \
		  $$KERNEL_H/qtextformat_p.h

	win32 {
		HEADERS    += \
		  $$KERNEL_H/qpaintengine_win.h \
 		  $$KERNEL_H/qpaintengine_win_p.h

 		SOURCES += \
		  $$KERNEL_CPP/qapplication_win.cpp \
		  $$KERNEL_CPP/qclipboard_win.cpp \
		  $$KERNEL_CPP/qcolor_win.cpp \
		  $$KERNEL_CPP/qcursor_win.cpp \
		  $$KERNEL_CPP/qdesktopwidget_win.cpp \
		  $$KERNEL_CPP/qdnd_win.cpp \
		  $$KERNEL_CPP/qfont_win.cpp \
		  $$KERNEL_CPP/qpaintengine_win.cpp \
		  $$KERNEL_CPP/qinputcontext_win.cpp \
		  $$KERNEL_CPP/qmime_win.cpp \
		  $$KERNEL_CPP/qpixmap_win.cpp \
		  $$KERNEL_CPP/qprinter_win.cpp \
		  $$KERNEL_CPP/qpaintdevice_win.cpp \
		  $$KERNEL_CPP/qregion_win.cpp \
		  $$KERNEL_CPP/qsound_win.cpp \
		  $$KERNEL_CPP/qwidget_win.cpp \
		  $$KERNEL_CPP/qole_win.c \
		  $$KERNEL_CPP/qfontengine_win.cpp \
		  $$KERNEL_CPP/qguieventloop_win.cpp
	}

	wince-* {
		SOURCES -= $$KERNEL_CPP/qfontengine_win.cpp \
			   $$KERNEL_CPP/qregion_win.cpp
		SOURCES += $$KERNEL_CPP/qfontengine_wce.cpp \
			   $$KERNEL_CPP/qregion_wce.cpp
		}


	unix:x11 {
	      HEADERS +=  $$KERNEL_H/qpaintengine_x11.h \
			  $$KERNEL_H/qpaintengine_x11_p.h \
			  $$KERNEL_H/qx11info_x11.h
	      SOURCES += $$KERNEL_CPP/qapplication_x11.cpp \
		          $$KERNEL_CPP/qclipboard_x11.cpp \
			  $$KERNEL_CPP/qcolor_x11.cpp \
			  $$KERNEL_CPP/qcursor_x11.cpp \
			  $$KERNEL_CPP/qdnd_x11.cpp \
			  $$KERNEL_CPP/qdesktopwidget_x11.cpp \
			  $$KERNEL_CPP/qguieventloop_x11.cpp \
			  $$KERNEL_CPP/qpaintengine_x11.cpp \
			  $$KERNEL_CPP/qfont_x11.cpp \
			  $$KERNEL_CPP/qinputcontext_x11.cpp \
			  $$KERNEL_CPP/qmotifdnd_x11.cpp \
			  $$KERNEL_CPP/qpixmap_x11.cpp \
			  $$KERNEL_CPP/qpaintdevice_x11.cpp \
			  $$KERNEL_CPP/qsound_x11.cpp \
			  $$KERNEL_CPP/qwidget_x11.cpp \
			  $$KERNEL_CPP/qwidgetcreate_x11.cpp \
		          $$KERNEL_CPP/qfontengine_x11.cpp \
			  $$KERNEL_CPP/qx11info_x11.cpp
	}

	!x11:mac {
	    exists(qsound_mac.cpp):SOURCES += $$KERNEL_CPP/qsound_mac.cpp
	    else:SOURCES += $$KERNEL_CPP/qsound_qws.cpp
	}
        !embedded:!x11:mac {
	      HEADERS += \
			  $$KERNEL_H/qpaintengine_mac.h \
			  $$KERNEL_H/qpaintengine_mac_p.h

	      SOURCES += $$KERNEL_CPP/qapplication_mac.cpp \
		          $$KERNEL_CPP/qclipboard_mac.cpp \
			  $$KERNEL_CPP/qcolor_mac.cpp \
			  $$KERNEL_CPP/qcursor_mac.cpp \
			  $$KERNEL_CPP/qmime_mac.cpp \
			  $$KERNEL_CPP/qdnd_mac.cpp \
			  $$KERNEL_CPP/qdesktopwidget_mac.cpp \
			  $$KERNEL_CPP/qpixmap_mac.cpp \
			  $$KERNEL_CPP/qprinter_mac.cpp \
			  $$KERNEL_CPP/qpaintdevice_mac.cpp \
			  $$KERNEL_CPP/qpaintengine_mac.cpp \
			  $$KERNEL_CPP/qwidget_mac.cpp \
			  $$KERNEL_CPP/qguieventloop_mac.cpp \
			  $$KERNEL_CPP/qfont_mac.cpp \
			  $$KERNEL_CPP/qfontengine_mac.cpp
        } else:unix {
	   SOURCES += $$KERNEL_CPP/qprinter_unix.cpp \
	              $$KERNEL_CPP/qpsprinter.cpp
        }

	unix:SOURCES += $$KERNEL_CPP/qregion_unix.cpp

	SOURCES += \
		  $$KERNEL_CPP/qabstractlayout.cpp \
		  $$KERNEL_CPP/qpaintengine.cpp \
		  $$KERNEL_CPP/qasyncimageio.cpp \
		  $$KERNEL_CPP/qaccel.cpp \
		  $$KERNEL_CPP/qapplication.cpp \
		  $$KERNEL_CPP/qbitmap.cpp \
		  $$KERNEL_CPP/qbrush.cpp \
		  $$KERNEL_CPP/qclipboard.cpp \
		  $$KERNEL_CPP/qcolor.cpp \
		  $$KERNEL_CPP/qcolor_p.cpp \
		  $$KERNEL_CPP/qcursor.cpp \
		  $$KERNEL_CPP/qdragobject.cpp \
		  $$KERNEL_CPP/qdrawutil.cpp \
		  $$KERNEL_CPP/qdropsite.cpp \
		  $$KERNEL_CPP/qevent.cpp \
		  $$KERNEL_CPP/qfont.cpp \
		  $$KERNEL_CPP/qfontdatabase.cpp \
		  $$KERNEL_CPP/qguieventloop.cpp \
		  $$KERNEL_CPP/qiconset.cpp \
		  $$KERNEL_CPP/qimage.cpp \
		  $$KERNEL_CPP/qimageformatplugin.cpp \
		  $$KERNEL_CPP/qimage_gui.cpp \
		  $$KERNEL_CPP/qkeysequence.cpp \
		  $$KERNEL_CPP/qlayout.cpp \
		  $$KERNEL_CPP/qlayoutengine.cpp \
		  $$KERNEL_CPP/qmime.cpp \
		  $$KERNEL_CPP/qmovie.cpp \
		  $$KERNEL_CPP/qpaintdevicemetrics.cpp \
		  $$KERNEL_CPP/qpainter.cpp \
		  $$KERNEL_CPP/qpalette.cpp \
		  $$KERNEL_CPP/qpen.cpp \
		  $$KERNEL_CPP/qpicture.cpp \
		  $$KERNEL_CPP/qpictureformatplugin.cpp \
		  $$KERNEL_CPP/qpixmap.cpp \
		  $$KERNEL_CPP/qpixmapcache.cpp \
		  $$KERNEL_CPP/qpoint.cpp \
		  $$KERNEL_CPP/qpointarray.cpp \
		  $$KERNEL_CPP/qpolygonscanner.cpp \
		  $$KERNEL_CPP/qprinter.cpp \
		  $$KERNEL_CPP/qrect.cpp \
		  $$KERNEL_CPP/qregion.cpp \
		  $$KERNEL_CPP/qrichtext.cpp \
		  $$KERNEL_CPP/qrichtext_p.cpp \
		  $$KERNEL_CPP/qscriptengine.cpp \
		  $$KERNEL_CPP/qsimplerichtext.cpp \
		  $$KERNEL_CPP/qsize.cpp \
		  $$KERNEL_CPP/qsizegrip.cpp \
		  $$KERNEL_CPP/qsound.cpp \
		  $$KERNEL_CPP/qstyle.cpp \
		  $$KERNEL_CPP/qstylesheet.cpp \
		  $$KERNEL_CPP/qtextengine.cpp \
		  $$KERNEL_CPP/qtextlayout.cpp \
		  $$KERNEL_CPP/qtextformat.cpp \
		  $$KERNEL_CPP/qvariant.cpp \
		  $$KERNEL_CPP/qwidget.cpp \
		  $$KERNEL_CPP/qwmatrix.cpp

	unix:HEADERS   += $$KERNEL_P/qpsprinter_p.h \
			  $$KERNEL_H/qfontdatabase.h

	embedded {
		HEADERS += \
			$$KERNEL_H/qpaintengine_qws.h \
			$$KERNEL_P/qpaintengine_qws_p.h

		SOURCES += \
			$$KERNEL_CPP/qfontengine_qws.cpp \
			$$KERNEL_CPP/qpaintengine_qws.cpp
	}

	wince-* {
	      HEADERS += $$KERNEL_H/qfunctions_wce.h
	      SOURCES += $$KERNEL_CPP/qfunctions_wce.cpp
	}

	x11 {
 	    CODECS_P		= codecs
 	    SOURCES += $$CODECS_CPP/qfontcncodec.cpp  \
	               $$CODECS_CPP/qfonthkcodec.cpp  \
	               $$CODECS_CPP/qfontjpcodec.cpp  \
		       $$CODECS_CPP/qfontkrcodec.cpp  \
		       $$CODECS_CPP/qfontlaocodec.cpp \
		       $$CODECS_CPP/qfonttwcodec.cpp
	    HEADERS += $$CODECS_P/qfontcodecs_p.h
 	}
}

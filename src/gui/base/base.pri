# Qt kernel module

# Only used on platforms with CONFIG += precompile_header
PRECOMPILED_HEADER = base/qt_gui_pch.h


KERNEL_P= kernel
HEADERS += \
	base/qabstractlayout.h \
	base/qaccel.h \
	base/qaction.h \
	base/qapplication.h \
	base/qapplication_p.h \
	base/qclipboard.h \
	base/qcursor.h \
	base/qdesktopwidget.h \
	base/qdragobject.h \
	base/qevent.h \
	base/qguieventloop.h\
	base/qguieventloop_p.h \
	base/qinputcontext_p.h \
	base/qkeysequence.h \
	base/qlayout.h \
	base/qmime.h \
	base/qsessionmanager.h \
	base/qsizepolicy.h \
	base/qsound.h \	
	base/qtooltip.h \
	base/qvariant.h \
	base/qwhatsthis.h \
	base/qwidget.h \
	base/qwindowdefs.h

SOURCES += \
	base/qabstractlayout.cpp \
	base/qaccel.cpp \
	base/qaction.cpp \
	base/qapplication.cpp \
	base/qclipboard.cpp \
	base/qcursor.cpp \
	base/qdragobject.cpp \
	base/qevent.cpp \
	base/qguieventloop.cpp \
	base/qkeysequence.cpp \
	base/qlayout.cpp \
	base/qlayoutengine.cpp \
	base/qmime.cpp \
	base/qpalette.cpp \
	base/qsound.cpp \
	base/qtooltip.cpp \
	base/qvariant.cpp \
	base/qwhatsthis.cpp \
	base/qwidget.cpp


win32 {
	SOURCES += \
		base/qapplication_win.cpp \
		base/qclipboard_win.cpp \
		base/qcursor_win.cpp \
		base/qdesktopwidget_win.cpp \
		base/qdnd_win.cpp \
		base/qinputcontext_win.cpp \
		base/qmime_win.cpp \
		base/qsound_win.cpp \
		base/qwidget_win.cpp \
		base/qole_win.c \
		base/qguieventloop_win.cpp
}

unix:x11 {
	HEADERS += \
		base/qx11info_x11.h

	SOURCES += \
		base/qapplication_x11.cpp \
		base/qclipboard_x11.cpp \
		base/qcursor_x11.cpp \
		base/qdnd_x11.cpp \
		base/qdesktopwidget_x11.cpp \
		base/qguieventloop_x11.cpp \
		base/qinputcontext_x11.cpp \
		base/qmotifdnd_x11.cpp \
		base/qsound_x11.cpp \
		base/qwidget_x11.cpp \
		base/qwidgetcreate_x11.cpp \
		base/qx11info_x11.cpp
}

embedded {
	SOURCES += \
		base/qapplication_qws.cpp \
		base/qclipboard_qws.cpp \
		base/qcursor_qws.cpp \
		base/qdesktopwidget_qws.cpp \
		base/qdnd_qws.cpp \
		base/qguieventloop_qws.cpp \
		base/qinputcontext_qws.cpp \
		base/qsound_qws.cpp \
		base/qwidget_qws.cpp
		
}

!x11:mac {
	exists(qsound_mac.cpp):SOURCES += base/qsound_mac.cpp
	else:SOURCES += base/qsound_qws.cpp
}

!embedded:!x11:mac {
	SOURCES += \
		base/qapplication_mac.cpp \
		base/qclipboard_mac.cpp \
		base/qcursor_mac.cpp \
		base/qmime_mac.cpp \
		base/qdnd_mac.cpp \
		base/qdesktopwidget_mac.cpp \
		base/qwidget_mac.cpp \
		base/qguieventloop_mac.cpp
}


wince-* {
			HEADERS += base/qfunctions_wce.h
			SOURCES += base/qfunctions_wce.cpp
}

# Qt kernel module

# Only used on platforms with CONFIG += precompile_header
PRECOMPILED_HEADER = kernel/qt_gui_pch.h


KERNEL_P= kernel
HEADERS += \
	kernel/qabstractlayout.h \
	kernel/qaccel.h \
	kernel/qaction.h \
	kernel/qapplication.h \
	kernel/qapplication_p.h \
	kernel/qclipboard.h \
	kernel/qcursor.h \
	kernel/qdesktopwidget.h \
	kernel/qdragobject.h \
	kernel/qdragobject_p.h \
	kernel/qevent.h \
	kernel/qguieventloop.h\
	kernel/qguieventloop_p.h \
	kernel/qinputcontext_p.h \
	kernel/qkeysequence.h \
	kernel/qlayout.h \
	kernel/qmime.h \
	kernel/qsessionmanager.h \
	kernel/qshortcut.h \
	kernel/qshortcutmap_p.h \
	kernel/qsizepolicy.h \
	kernel/qsound.h \	
	kernel/qstackedlayout.h \	
	kernel/qtooltip.h \
	kernel/qvariant.h \
	kernel/qwhatsthis.h \
	kernel/qwidget.h \
	kernel/qwindowdefs.h

SOURCES += \
	kernel/qabstractlayout.cpp \
	kernel/qaccel.cpp \
	kernel/qaction.cpp \
	kernel/qapplication.cpp \
	kernel/qclipboard.cpp \
	kernel/qcursor.cpp \
	kernel/qdragobject.cpp \
	kernel/qevent.cpp \
	kernel/qguieventloop.cpp \
	kernel/qkeysequence.cpp \
	kernel/qlayout.cpp \
	kernel/qlayoutengine.cpp \
	kernel/qmime.cpp \
	kernel/qpalette.cpp \
	kernel/qshortcut.cpp \
	kernel/qshortcutmap.cpp \
	kernel/qsound.cpp \
	kernel/qstackedlayout.cpp \	
	kernel/qtooltip.cpp \
	kernel/qvariant.cpp \
	kernel/qwhatsthis.cpp \
	kernel/qwidget.cpp


win32 {
	SOURCES += \
		kernel/qapplication_win.cpp \
		kernel/qclipboard_win.cpp \
		kernel/qcursor_win.cpp \
		kernel/qdesktopwidget_win.cpp \
		kernel/qdnd_win.cpp \
		kernel/qinputcontext_win.cpp \
		kernel/qmime_win.cpp \
		kernel/qsound_win.cpp \
		kernel/qwidget_win.cpp \
		kernel/qole_win.c \
		kernel/qguieventloop_win.cpp
}

unix:x11 {
	HEADERS += \
		kernel/qx11info_x11.h

	SOURCES += \
		kernel/qapplication_x11.cpp \
		kernel/qclipboard_x11.cpp \
		kernel/qcursor_x11.cpp \
		kernel/qdnd_x11.cpp \
		kernel/qdesktopwidget_x11.cpp \
		kernel/qguieventloop_x11.cpp \
		kernel/qinputcontext_x11.cpp \
		kernel/qmotifdnd_x11.cpp \
		kernel/qsound_x11.cpp \
		kernel/qwidget_x11.cpp \
		kernel/qwidgetcreate_x11.cpp \
		kernel/qx11info_x11.cpp
}

embedded {
	SOURCES += \
		kernel/qapplication_qws.cpp \
		kernel/qclipboard_qws.cpp \
		kernel/qcursor_qws.cpp \
		kernel/qdesktopwidget_qws.cpp \
		kernel/qdnd_qws.cpp \
		kernel/qguieventloop_qws.cpp \
		kernel/qinputcontext_qws.cpp \
		kernel/qsound_qws.cpp \
		kernel/qwidget_qws.cpp
		
}

!x11:mac {
	exists(qsound_mac.cpp):SOURCES += kernel/qsound_mac.cpp
	else:SOURCES += kernel/qsound_qws.cpp
}

!embedded:!x11:mac {
	SOURCES += \
		kernel/qapplication_mac.cpp \
		kernel/qclipboard_mac.cpp \
		kernel/qcursor_mac.cpp \
		kernel/qmime_mac.cpp \
		kernel/qdnd_mac.cpp \
		kernel/qdesktopwidget_mac.cpp \
		kernel/qwidget_mac.cpp \
		kernel/qguieventloop_mac.cpp
}


wince-* {
	HEADERS += kernel/qfunctions_wce.h
	SOURCES += kernel/qfunctions_wce.cpp
}

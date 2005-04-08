# Qt kernel module

# Only used on platforms with CONFIG += precompile_header
PRECOMPILED_HEADER = kernel/qt_gui_pch.h


KERNEL_P= kernel
HEADERS += \
	kernel/qaction.h \
	kernel/qactiongroup.h \
	kernel/qapplication.h \
	kernel/qapplication_p.h \
	kernel/qboxlayout.h \
	kernel/qclipboard.h \
	kernel/qcursor.h \
	kernel/qdesktopwidget.h \
	kernel/qdrag.h \
	kernel/qdnd_p.h \
	kernel/qevent.h \
	kernel/qgridlayout.h \
	kernel/qkeysequence.h \
	kernel/qlayout.h \
	kernel/qlayoutitem.h \
	kernel/qmime.h \
	kernel/qsessionmanager.h \
	kernel/qshortcut.h \
	kernel/qshortcutmap_p.h \
	kernel/qsizepolicy.h \
	kernel/qsound.h \
	kernel/qsound_p.h \
	kernel/qstackedlayout.h \
	kernel/qtooltip.h \
	kernel/qwhatsthis.h \
	kernel/qwidget.h \
	kernel/qwindowdefs.h

SOURCES += \
	kernel/qaction.cpp \
	kernel/qactiongroup.cpp \
	kernel/qapplication.cpp \
	kernel/qboxlayout.cpp \
	kernel/qclipboard.cpp \
	kernel/qcursor.cpp \
	kernel/qdrag.cpp \
	kernel/qdnd.cpp \
	kernel/qevent.cpp \
	kernel/qgridlayout.cpp \
	kernel/qkeysequence.cpp \
	kernel/qlayout.cpp \
	kernel/qlayoutengine.cpp \
	kernel/qlayoutitem.cpp \
	kernel/qmime.cpp \
	kernel/qpalette.cpp \
	kernel/qshortcut.cpp \
	kernel/qshortcutmap.cpp \
	kernel/qsound.cpp \
	kernel/qstackedlayout.cpp \
	kernel/qtooltip.cpp \
	kernel/qguivariant.cpp \
	kernel/qwhatsthis.cpp \
	kernel/qwidget.cpp


MOCABLE += kernel/qsizepolicy.h \
	kernel/qpalette.h

win32 {
	SOURCES += \
		kernel/qapplication_win.cpp \
		kernel/qclipboard_win.cpp \
		kernel/qcursor_win.cpp \
		kernel/qdesktopwidget_win.cpp \
		kernel/qdnd_win.cpp \
		kernel/qmime_win.cpp \
		kernel/qsound_win.cpp \
		kernel/qwidget_win.cpp \
		kernel/qole_win.c
}

unix:x11 {
	HEADERS += \
		kernel/qeventdispatcher_x11_p.h \
		kernel/qx11info_x11.h

	SOURCES += \
		kernel/qapplication_x11.cpp \
		kernel/qclipboard_x11.cpp \
		kernel/qcursor_x11.cpp \
		kernel/qdnd_x11.cpp \
		kernel/qdesktopwidget_x11.cpp \
		kernel/qeventdispatcher_x11.cpp \
		kernel/qmotifdnd_x11.cpp \
		kernel/qsound_x11.cpp \
		kernel/qwidget_x11.cpp \
		kernel/qwidgetcreate_x11.cpp \
		kernel/qx11info_x11.cpp
}

embedded {
	HEADERS += \
		kernel/qeventdispatcher_qws_p.h

	SOURCES += \
		kernel/qapplication_qws.cpp \
		kernel/qclipboard_qws.cpp \
		kernel/qcursor_qws.cpp \
		kernel/qdesktopwidget_qws.cpp \
		kernel/qdnd_qws.cpp \
		kernel/qeventdispatcher_qws.cpp \
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
                kernel/qeventdispatcher_mac.cpp \
		kernel/qmime_mac.cpp \
		kernel/qdnd_mac.cpp \
		kernel/qdesktopwidget_mac.cpp \
		kernel/qwidget_mac.cpp
        HEADERS += \
                kernel/qeventdispatcher_mac_p.h
}

wince-* {
	HEADERS += kernel/qfunctions_wce.h
	SOURCES += kernel/qfunctions_wce.cpp
}

# Qt core object module

HEADERS += \
	kernel/qabstracteventdispatcher.h \
	kernel/qbasictimer.h \
	kernel/qeventloop.h\
	kernel/qpointer.h \
	kernel/qcoreapplication.h \
	kernel/qcoreevent.h \
	kernel/qcorevariant.h \
	kernel/qmetaobject.h \
	kernel/qmetatype.h \
        kernel/qmimedata.h \
	kernel/qobject.h \
	kernel/qobjectdefs.h \
	kernel/qsignal.h \
	kernel/qsignalmapper.h \
	kernel/qsocketnotifier.h \
	kernel/qtimer.h \
	kernel/qtranslator.h \
	kernel/qinternal_p.h \
	kernel/qabstracteventdispatcher_p.h \
	kernel/qcoreapplication_p.h \
        kernel/qcorevariant_p.h \
	kernel/qobjectcleanuphandler.h

SOURCES += \
	kernel/qabstracteventdispatcher.cpp \
	kernel/qbasictimer.cpp \
	kernel/qeventloop.cpp \
	kernel/qinternal.cpp \
	kernel/qcoreapplication.cpp \
	kernel/qcoreevent.cpp \
	kernel/qcorevariant.cpp \
	kernel/qmetaobject.cpp \
	kernel/qmetatype.cpp \
        kernel/qmimedata.cpp \
	kernel/qobject.cpp \
	kernel/qobjectcleanuphandler.cpp \
	kernel/qsignal.cpp \
	kernel/qsignalmapper.cpp \
	kernel/qsocketnotifier.cpp \
	kernel/qtimer.cpp \
	kernel/qtranslator.cpp

win32 {
	SOURCES += \
		kernel/qeventdispatcher_win.cpp \
		kernel/qcoreapplication_win.cpp \
		kernel/qwineventnotifier_p.cpp
	HEADERS += \
		kernel/qeventdispatcher_win.h \
		kernel/qwineventnotifier_p.h
}

mac {
       SOURCES += \
		kernel/qcore_mac.cpp \
                kernel/qcoreapplication_mac.cpp
}

unix {
	SOURCES += \
		kernel/qcrashhandler.cpp \
		kernel/qeventdispatcher_unix.cpp
	HEADERS += \
		kernel/qcrashhandler_p.h \
		kernel/qeventdispatcher_unix.h
}

embedded:SOURCES += kernel/qsharedmemory_p.cpp


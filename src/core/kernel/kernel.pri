# Qt core object module

HEADERS += \
	kernel/qbasictimer.h \
	kernel/qeventloop.h\
	kernel/qpointer.h \
	kernel/qcoreapplication.h \
	kernel/qcoreevent.h \
	kernel/qcorevariant.h \
	kernel/qmetaobject.h \
	kernel/qobject.h \
	kernel/qobjectdefs.h \
	kernel/qprocess.h \
	kernel/qsignal.h \
	kernel/qsignalmapper.h \
	kernel/qsocketnotifier.h \
	kernel/qtimer.h \
	kernel/qtranslator.h \
	kernel/qeventloop_p.h \
	kernel/qinternal_p.h \
	kernel/qcoreapplication_p.h \
	kernel/qobjectcleanuphandler.h

SOURCES += \
	kernel/qbasictimer.cpp \
	kernel/qeventloop.cpp \
	kernel/qinternal.cpp \
	kernel/qcoreapplication.cpp \
	kernel/qcoreevent.cpp \
	kernel/qcorevariant.cpp \
	kernel/qmetaobject.cpp \
	kernel/qobject.cpp \
	kernel/qobjectcleanuphandler.cpp \
	kernel/qprocess.cpp \
	kernel/qsignal.cpp \
	kernel/qsignalmapper.cpp \
	kernel/qsocketnotifier.cpp \
	kernel/qtimer.cpp \
	kernel/qtranslator.cpp \
	
win32 {
	SOURCES += \
		kernel/qprocess_win.cpp \
		kernel/qeventloop_win.cpp \
		kernel/qcoreapplication_win.cpp
} 

mac {
       SOURCES += \
		kernel/qcore_mac.cpp \
                kernel/qcoreapplication_mac.cpp
}

unix {
	SOURCES += \
		kernel/qcrashhandler.cpp \
		kernel/qprocess_unix.cpp \
		kernel/qeventloop_unix.cpp
	HEADERS += \
		kernel/qcrashhandler_p.h
}

embedded:SOURCES += kernel/qsharedmemory_p.cpp


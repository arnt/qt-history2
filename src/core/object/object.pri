# Qt core object module

HEADERS += \
	object/qbasictimer.h \
	object/qeventloop.h\
	object/qpointer.h \
	object/qguardedptr.h \
	object/qcoreapplication.h \
	object/qcoreevent.h \
	object/qcorevariant.h \
	object/qmetaobject.h \
	object/qobject.h \
	object/qobjectdefs.h \
	object/qprocess.h \
	object/qsignal.h \
	object/qsignalmapper.h \
	object/qsocketnotifier.h \
	object/qtimer.h \
	object/qtranslator.h \
	object/qurl.h \
	object/qeventloop_p.h \
	object/qinternal_p.h \
	object/qcoreapplication_p.h \
	object/qobjectcleanuphandler.h

SOURCES += \
	object/qbasictimer.cpp \
	object/qeventloop.cpp \
	object/qinternal.cpp \
	object/qcoreapplication.cpp \
	object/qcoreevent.cpp \
	object/qcorevariant.cpp \
	object/qmetaobject.cpp \
	object/qobject.cpp \
	object/qobjectcleanuphandler.cpp \
	object/qprocess.cpp \
	object/qsignal.cpp \
	object/qsignalmapper.cpp \
	object/qsocketnotifier.cpp \
	object/qtimer.cpp \
	object/qtranslator.cpp \
	object/qurl.cpp
	
win32 {
	SOURCES += \
		object/qprocess_win.cpp \
		object/qeventloop_win.cpp \
		object/qcoreapplication_win.cpp
}

unix {
	SOURCES += \
		object/qprocess_unix.cpp \
		object/qeventloop_unix.cpp
}

embedded:SOURCES += object/qsharedmemory_p.cpp


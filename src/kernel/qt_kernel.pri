# Qt kernel module

kernel {
	KERNEL_P	= kernel
	HEADERS += \
		  $$KERNEL_H/qbasictimer.h \
		  $$KERNEL_H/qeventloop.h\
		  $$KERNEL_H/qgplugin.h \
		  $$KERNEL_H/qpointer.h \
		  $$KERNEL_H/qguardedptr.h \
		  $$KERNEL_H/qkernelapplication.h \
		  $$KERNEL_H/qkernelevent.h \
		  $$KERNEL_H/qkernelvariant.h \
		  $$KERNEL_H/qmetaobject.h \
		  $$KERNEL_H/qnamespace.h \
		  $$KERNEL_H/qobject.h \
		  $$KERNEL_H/qobjectdefs.h \
		  $$KERNEL_H/qobjectdict.h \
		  $$KERNEL_H/qprocess.h \
		  $$KERNEL_H/qsignal.h \
		  $$KERNEL_H/qsignalmapper.h \
		  $$KERNEL_H/qsocketnotifier.h \
		  $$KERNEL_H/qtimer.h \
		  $$KERNEL_H/qtranslator.h \
		  $$KERNEL_H/qurl.h \
		  $$KERNEL_P/qeventloop_p.h \
		  $$KERNEL_P/qinternal_p.h \
		  $$KERNEL_P/qkernelapplication_p.h \
 		  $$KERNEL_H/qobjectcleanuphandler.h


		  
        win32:SOURCES += $$KERNEL_CPP/qprocess_win.cpp \
			 $$KERNEL_CPP/qeventloop_win.cpp \
			 $$KERNEL_CPP/qkernelapplication_win.cpp

	unix:SOURCES += $$KERNEL_CPP/qprocess_unix.cpp \
			$$KERNEL_CPP/qeventloop_unix.cpp

	SOURCES += \
		  $$KERNEL_CPP/qbasictimer.cpp \
		  $$KERNEL_CPP/qeventloop.cpp \
		  $$KERNEL_CPP/qgplugin.cpp \
		  $$KERNEL_CPP/qinternal.cpp \
		  $$KERNEL_CPP/qkernelapplication.cpp \
		  $$KERNEL_CPP/qkernelevent.cpp \
		  $$KERNEL_CPP/qkernelvariant.cpp \
		  $$KERNEL_CPP/qmetaobject.cpp \
		  $$KERNEL_CPP/qobject.cpp \
		  $$KERNEL_CPP/qobjectcleanuphandler.cpp \
		  $$KERNEL_CPP/qprocess.cpp \
		  $$KERNEL_CPP/qsignal.cpp \
		  $$KERNEL_CPP/qsignalmapper.cpp \
		  $$KERNEL_CPP/qsocketnotifier.cpp \
		  $$KERNEL_CPP/qtimer.cpp \
		  $$KERNEL_CPP/qtranslator.cpp \
		  $$KERNEL_CPP/qurl.cpp
	

	embedded:SOURCES += $$KERNEL_CPP/qsharedmemory_p.cpp
}

#zlib support
zlib {
	INCLUDEPATH       += 3rdparty/zlib
	SOURCES	+= 3rdparty/zlib/adler32.c \
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
}
!no-zlib:!zlib {
   unix:LIBS += -lz
   win32:LIBS += libz.lib
}



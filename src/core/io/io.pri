# Qt core io module

HEADERS +=  \
	io/qbuffer.h \
	io/qdatastream.h \
	io/qdebug.h \
	io/qdir.h \
	io/qfile.h \
	io/qfileinfo.h \
        io/qfileinfoengine.h \
        io/qfileinfoengine_p.h \
        io/qfileengine.h \
        io/qfileengine_p.h \
        io/qdirengine.h \
        io/qdirengine_p.h \
	io/qiodevice.h \
	io/qiodevice_p.h \
	io/qtextstream.h \
	io/qsettings.h \
	io/qsettings_p.h \
        io/qresource.h \
        io/qresourceengine_p.h \
	io/qurl.h

SOURCES += \
	io/qbuffer.cpp \
	io/qdatastream.cpp \
	io/qdebug.cpp \
	io/qdir.cpp \
	io/qfile.cpp \
	io/qfileinfo.cpp \
        io/qfsfileinfoengine.cpp \
        io/qfsfileengine.cpp \
        io/qfsdirengine.cpp \
	io/qiodevice.cpp \
	io/qtextstream.cpp \
	io/qsettings.cpp \
        io/qresource.cpp \
        io/qresourceengine.cpp \
	io/qurl.cpp

win32 {
	wince-* {
		SOURCES += \
			io/qfsdirengine_wce.cpp \
			io/qfsfileengine_wce.cpp \
			io/qfsfileinfoengine_wce.cpp
	} else {
		SOURCES += \
			io/qfsdirengine_win.cpp \
			io/qfsfileengine_win.cpp \
			io/qfsfileinfoengine_win.cpp
	}
	SOURCES += io/qsettings_win.cpp
} else:unix {
	SOURCES += \
		io/qfsdirengine_unix.cpp \
		io/qfsfileengine_unix.cpp \
		io/qfsfileinfoengine_unix.cpp
	mac:SOURCES += io/qsettings_mac.cpp
}

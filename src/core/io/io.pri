# Qt core io module

HEADERS +=  \
	io/qbuffer.h \
	io/qdatastream.h \
	io/qdebug.h \
	io/qdir.h \
	io/qfile.h \
	io/qfileinfo.h \
        io/qfileengine.h \
        io/qfileengine_p.h \
        io/qioengine.h \
        io/qioengine_p.h \
	io/qiodevice.h \
	io/qiodevice_p.h \
	io/qtextstream.h \
	io/qtemporaryfile.h \
        io/qresource.h \
        io/qresourceengine_p.h \
	io/qurl.h \
	io/qcoresettings.h \
	io/qcoresettings_p.h


SOURCES += \
	io/qbuffer.cpp \
	io/qdatastream.cpp \
	io/qdebug.cpp \
	io/qdir.cpp \
	io/qfile.cpp \
	io/qfileinfo.cpp \
        io/qfileengine.cpp \
	io/qiodevice.cpp \
	io/qioengine.cpp \
	io/qtextstream.cpp \
	io/qtemporaryfile.cpp \
        io/qresource.cpp \
        io/qresourceengine.cpp \
	io/qurl.cpp \
	io/qcoresettings.cpp

win32 {
	wince-*:SOURCES += io/qfileengine_wce.cpp
	else:SOURCES += io/qfileengine_win.cpp 
	SOURCES += io/qcoresettings_win.cpp
} else:unix {
	SOURCES += io/qfileengine_unix.cpp 
	mac:SOURCES += io/qcoresettings_mac.cpp
}

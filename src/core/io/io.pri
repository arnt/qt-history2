# Qt core io module

HEADERS +=  \
	io/qbuffer.h \
	io/qdatastream.h \
	io/qdebug.h \
	io/qdir.h \
	io/qdir_p.h \
	io/qfile.h \
	io/qfiledefs_p.h \
	io/qfileinfo.h \
	io/qiodevice.h \
	io/qiodevice_p.h \
	io/qtextstream.h \
	io/qsettings.h \
	io/qsettings_p.h \
	io/qurl.h

SOURCES += \
	io/qbuffer.cpp \
	io/qdatastream.cpp \
	io/qdebug.cpp \
	io/qdir.cpp \
	io/qfile.cpp \
	io/qfileinfo.cpp \
	io/qiodevice.cpp \
	io/qtextstream.cpp \
	io/qsettings.cpp \
	io/qurl.cpp

win32 {
	wince-* {
		SOURCES += \
			io/qdir_wce.cpp \
			io/qfile_wce.cpp \
			io/qfileinfo_wce.cpp
	} else {
		SOURCES += \
			io/qdir_win.cpp \
			io/qfile_win.cpp \
			io/qfileinfo_win.cpp
	}
	SOURCES += io/qsettings_win.cpp
} else:unix {
	SOURCES += \
		io/qdir_unix.cpp \
		io/qfile_unix.cpp \
		io/qfileinfo_unix.cpp
	mac:SOURCES += io/qsettings_mac.cpp
}

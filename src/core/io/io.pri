# Qt core io module

HEADERS +=  \
	io/qbuffer.h \
	io/qdatastream.h \
	io/qdir.h \
	io/qdir_p.h \
	io/qfile.h \
	io/qfiledefs_p.h \
	io/qfileinfo.h \
	io/qiodevice.h \
	io/qtextstream.h 

SOURCES += \
	io/qbuffer.cpp \
	io/qdatastream.cpp \
	io/qdir.cpp \
	io/qfile.cpp \
	io/qfileinfo.cpp \
	io/qiodevice.cpp \
	io/qtextstream.cpp

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
} else:unix {
	SOURCES += \
		io/qdir_unix.cpp \
		io/qfile_unix.cpp \
		io/qfileinfo_unix.cpp
} 

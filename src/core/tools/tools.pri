# Qt tools module

HEADERS +=  \
	tools/qalgorithms.h \
	tools/qbitarray.h \
	tools/qbytearray.h \
	tools/qcache.h \
	tools/qchar.h \
	tools/qdatetime.h \
	tools/qhash.h \
	tools/qlinkedlist.h \
	tools/qlist.h \
	tools/qlocale.h \
	tools/qlocale_p.h \
	tools/qmap.h \
	tools/qqueue.h \
	tools/qregexp.h \
	tools/qshareddatapointer.h \
	tools/qstack.h \
	tools/qstring.h \
	tools/qstringlist.h \
	tools/qunicodetables_p.h \
	tools/qvarlengtharray.h \
	tools/qvector.h


SOURCES += \
	tools/qbitarray.cpp \
	tools/qbytearray.cpp \
	tools/qchar.cpp \
	tools/qdatetime.cpp \
	tools/qhash.cpp \
	tools/qlinkedlist.cpp \
	tools/qlist.cpp \
	tools/qlocale.cpp \
	tools/qmap.cpp \
	tools/qqueue.cpp \
	tools/qregexp.cpp \
	tools/qshareddatapointer.cpp \
	tools/qstack.cpp \
	tools/qstring.cpp \
	tools/qstringlist.cpp \
	tools/qunicodetables.cpp \
	tools/qvector.cpp


#zlib support
zlib {
INCLUDEPATH += ../3rdparty/zlib
	SOURCES+= \
		../3rdparty/zlib/adler32.c \
		../3rdparty/zlib/compress.c \
		../3rdparty/zlib/crc32.c \
		../3rdparty/zlib/deflate.c \
		../3rdparty/zlib/gzio.c \
		../3rdparty/zlib/inffast.c \
		../3rdparty/zlib/inflate.c \
		../3rdparty/zlib/inftrees.c \
		../3rdparty/zlib/trees.c \
		../3rdparty/zlib/uncompr.c \
		../3rdparty/zlib/zutil.c
}
!no-zlib:!zlib {
	unix:LIBS += -lz
	win32:LIBS += libz.lib
}



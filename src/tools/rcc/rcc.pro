TARGET = rcc
CONFIG += console
DESTDIR = ../../../bin
mac:CONFIG -= resource_fork

DEFINES += QT_NO_TEXTCODEC QT_NO_COMPONENT QT_NO_STL QT_NO_UNICODETABLES QT_NODLL QT_BUILD_RCC QT_NO_THREAD
CONFIG -= qt
QT = 

#zlib support
contains(QT_CONFIG, zlib) {
    INCLUDEPATH += $(QTDIR)/3rdparty/zlib
    SOURCES+= \
     ../../3rdparty/zlib/adler32.c \
     ../../3rdparty/zlib/compress.c \
     ../../3rdparty/zlib/crc32.c \
     ../../3rdparty/zlib/deflate.c \
     ../../3rdparty/zlib/gzio.c \
     ../../3rdparty/zlib/inffast.c \
     ../../3rdparty/zlib/inflate.c \
     ../../3rdparty/zlib/inftrees.c \
     ../../3rdparty/zlib/trees.c \
     ../../3rdparty/zlib/uncompr.c \
     ../../3rdparty/zlib/zutil.c
} else {
    unix:LIBS += -lz
    #win32:LIBS += libz.lib
}

#qt files (bootstrap
INCLUDEPATH += $$QT_SOURCE_TREE/include/QtCore
SOURCES += \
           #tools
           ../../core/tools/qbytearray.cpp ../../core/tools/qdatetime.cpp \
           ../../core/tools/qstring.cpp ../../core/tools/qlist.cpp \
           ../../core/tools/qchar.cpp ../../core/tools/qlocale.cpp \
           ../../core/tools/qunicodetables.cpp ../../core/tools/qhash.cpp \
           ../../core/tools/qregexp.cpp ../../core/tools/qvector.cpp \ 
           ../../core/tools/qmap.cpp ../../core/tools/qstringmatcher.cpp \ 
           ../../core/tools/qbytearraymatcher.cpp ../../core/tools/qbitarray.cpp \
           #io
           ../../core/io/qfile.cpp ../../core/io/qdir.cpp ../../core/io/qfileinfo.cpp \
           ../../core/io/qfileengine.cpp ../../core/io/qdatastream.cpp ../../core/io/qbuffer.cpp \
           ../../core/io/qiodevice.cpp ../../core/io/qtextstream.cpp ../../core/io/qioengine.cpp \
           #global
           ../../core/global/qglobal.cpp
#platform specific
win32:SOURCES += ../../core/io/qfileengine_win.cpp
else:unix:SOURCES += ../../core/io/qfileengine_unix.cpp
mac:LIBS += -framework ApplicationServices

#actual stuff in here
SOURCES += main.cpp

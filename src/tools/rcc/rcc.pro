TARGET = rcc
CONFIG += console
DESTDIR = ../../../bin
mac:CONFIG -= resource_fork
build_all:CONFIG += release

DEFINES += QT_NO_CODECS QT_NO_COMPONENT QT_NO_STL QT_NO_UNICODETABLES QT_NODLL \
           QT_BUILD_RCC QT_NO_THREAD QT_NO_QOBJECT
CONFIG -= qt
QT =

#zlib support
contains(QT_CONFIG, zlib) {
    INCLUDEPATH += $(QTDIR)/src/3rdparty/zlib
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

#qt files (bootstrap)
INCLUDEPATH += $$QT_BUILD_TREE/src/core/arch/generic \
               $$QT_BUILD_TREE/include/QtCore $$QT_BUILD_TREE/include/QtXml
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
           ../../core/io/qiodevice.cpp ../../core/io/qtextstream.cpp ../../core/io/qtemporaryfile.cpp \
           #xml
           ../../xml/qxml.cpp ../../xml/qdom.cpp \
           #codecs
           ../../core/codecs/qtextcodec.cpp ../../core/codecs/qutfcodec.cpp \
           #global
           ../../core/global/qglobal.cpp 
#platform specific
win32:SOURCES += ../../core/io/qfileengine_win.cpp
else:unix:SOURCES += ../../core/io/qfileengine_unix.cpp
mac:LIBS += -framework ApplicationServices
mac:SOURCES += ../../core/kernel/qcore_mac.cpp

#actual stuff in here
SOURCES += main.cpp

target.path=$$bins.path
INSTALLS += target

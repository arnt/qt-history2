TARGET = resc
CONFIG += console
DESTDIR = ../bin
mac:CONFIG -= resource_fork

CONFIG += bootstrap
bootstrap {
   DEFINES += QT_NO_TEXTCODEC QT_NO_COMPONENT QT_NO_STL QT_NO_UNICODETABLES QT_NODLL QT_BUILD_RESC QT_NO_THREAD
   CONFIG -= qt
   QT = 

   #zlib support
   contains(QT_CONFIG, zlib) {
      VPATH_SOURCES += $(QTDIR)/src/3rdparty/zlib
      INCLUDEPATH += $(QTDIR)/src/3rdparty/zlib
      SOURCES+= \
         adler32.c \
	 compress.c \
	 crc32.c \
	 deflate.c \
	 gzio.c \
	 inffast.c \
	 inflate.c \
	 inftrees.c \
	 trees.c \
	 uncompr.c \
	 zutil.c
   } else {
     unix:LIBS += -lz
     #win32:LIBS += libz.lib
   }

   VPATH_SOURCES += $(QTDIR)/src/core/io $(QTDIR)/src/core/tools $(QTDIR)/src/core/global
   INCLUDEPATH += $(QTDIR)/include/QtCore
   SOURCES += qbytearray.cpp qfile.cpp qdir.cpp qfileinfo.cpp \
            qstring.cpp qiodevice.cpp qfsfileengine.cpp qfsfileinfoengine.cpp \
            qfsdirengine.cpp qtextstream.cpp qlist.cpp qglobal.cpp qdatetime.cpp \
            qchar.cpp qlocale.cpp qunicodetables.cpp qregexp.cpp qvector.cpp qhash.cpp \
            qmap.cpp qstringmatcher.cpp qbytearraymatcher.cpp qdatastream.cpp qbuffer.cpp \
            qbitarray.cpp
   win32:SOURCES += qfsfileinfoengine_win.cpp qfsfileengine_win.cpp qfsdirengine_win.cpp
   else:unix:SOURCES += qfsfileinfoengine_unix.cpp qfsfileengine_unix.cpp qfsdirengine_unix.cpp
   mac:LIBS += -framework ApplicationServices
} else {
   QT = core
}

#my stuff
SOURCES += main.cpp

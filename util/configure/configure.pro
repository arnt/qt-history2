CONFIG = console release
DEFINES = QT_NODLL QT_NO_CODECS QT_LITE_COMPONENT QT_NO_STL
HEADERS	 = configureapp.h \
           $$QT_SOURCE_TREE/src/tools/qdir.h \
           $$QT_SOURCE_TREE/src/tools/qstring.h \
           $$QT_SOURCE_TREE/src/tools/qfileinfo.h \
           $$QT_SOURCE_TREE/src/tools/qfile.h \
           $$QT_SOURCE_TREE/src/tools/qiodevice.h \
           $$QT_SOURCE_TREE/src/tools/qregexp.h \
           $$QT_SOURCE_TREE/src/tools/qgcache.h \
           $$QT_SOURCE_TREE/src/tools/qbitarray.h \
           $$QT_SOURCE_TREE/src/tools/qgdict.h \
           $$QT_SOURCE_TREE/src/tools/qgvector.h \
           $$QT_SOURCE_TREE/src/tools/qdatastream.h \ 
           $$QT_SOURCE_TREE/src/tools/qglist.h \
           $$QT_SOURCE_TREE/src/tools/qgarray.h \ 
           $$QT_SOURCE_TREE/src/tools/qglobal.h \ 
           $$QT_SOURCE_TREE/src/tools/qptrcollection.h \
           $$QT_SOURCE_TREE/src/tools/qbuffer.h \
           $$QT_SOURCE_TREE/src/tools/qstringlist.h \
           $$QT_SOURCE_TREE/src/tools/qcstring.h \
           $$QT_SOURCE_TREE/src/tools/qtextstream.h \
           $$QT_SOURCE_TREE/src/tools/qdatetime.h \
           $$QT_SOURCE_TREE/src/codecs/qtextcodec.h \
           $$QT_SOURCE_TREE/src/tools/qmap.h \
           $$QT_SOURCE_TREE/src/tools/qlibrary.h

SOURCES	 = main.cpp configureapp.cpp \
           $$QT_SOURCE_TREE/src/tools/qdir.cpp \
           $$QT_SOURCE_TREE/src/tools/qdir_win.cpp \
           $$QT_SOURCE_TREE/src/tools/qstring.cpp \
           $$QT_SOURCE_TREE/src/tools/qfileinfo.cpp \
           $$QT_SOURCE_TREE/src/tools/qfileinfo_win.cpp \
           $$QT_SOURCE_TREE/src/tools/qfile.cpp \
           $$QT_SOURCE_TREE/src/tools/qfile_win.cpp \
           $$QT_SOURCE_TREE/src/tools/qiodevice.cpp \
           $$QT_SOURCE_TREE/src/tools/qregexp.cpp \
           $$QT_SOURCE_TREE/src/tools/qgcache.cpp \
           $$QT_SOURCE_TREE/src/tools/qbitarray.cpp \
           $$QT_SOURCE_TREE/src/tools/qgdict.cpp \
           $$QT_SOURCE_TREE/src/tools/qgvector.cpp \
           $$QT_SOURCE_TREE/src/tools/qdatastream.cpp \
           $$QT_SOURCE_TREE/src/tools/qglist.cpp \
           $$QT_SOURCE_TREE/src/tools/qgarray.cpp \
           $$QT_SOURCE_TREE/src/tools/qglobal.cpp \
           $$QT_SOURCE_TREE/src/tools/qptrcollection.cpp \
           $$QT_SOURCE_TREE/src/tools/qbuffer.cpp \
           $$QT_SOURCE_TREE/src/tools/qstringlist.cpp \
           $$QT_SOURCE_TREE/src/tools/qcstring.cpp \
           $$QT_SOURCE_TREE/src/tools/qtextstream.cpp \
           $$QT_SOURCE_TREE/src/tools/qdatetime.cpp \
           $$QT_SOURCE_TREE/src/codecs/qtextcodec.cpp \
           $$QT_SOURCE_TREE/src/tools/qmap.cpp \
           $$QT_SOURCE_TREE/src/tools/qlibrary.cpp \
           $$QT_SOURCE_TREE/src/tools/qlibrary_win.cpp

INCLUDEPATH += $$QT_SOURCE_TREE/include/ $$QT_SOURCE_TREE/src/tools/
INTERFACES=
TARGET=configure
DESTDIR=../../dist/win/bin
LIBS = ole32.lib

win32:CONFIG+=console

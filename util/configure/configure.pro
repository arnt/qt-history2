CONFIG = console debug
DEFINES = QT_NODLL QT_NO_CODECS QT_NO_TEXTCODEC QT_NO_UNICODETABLES QT_LITE_COMPONENT QT_NO_STL QT_NO_COMPRESS
HEADERS	 = configureapp.h \
           $$QT_SOURCE_TREE/src/tools/qdir.h \
           $$QT_SOURCE_TREE/src/tools/qstring.h \
           $$QT_SOURCE_TREE/src/tools/qfileinfo.h \
           $$QT_SOURCE_TREE/src/tools/qfile.h \
           $$QT_SOURCE_TREE/src/tools/qiodevice.h \
           $$QT_SOURCE_TREE/src/tools/qregexp.h \
           $$QT_SOURCE_TREE/src/compat/qgcache.h \
           $$QT_SOURCE_TREE/src/tools/qbitarray.h \
	   $$QT_SOURCE_TREE/src/tools/qbytearray.h \
	   $$QT_SOURCE_TREE/src/tools/qchar.h \
           $$QT_SOURCE_TREE/src/compat/qgdict.h \
           $$QT_SOURCE_TREE/src/compat/qgvector.h \
           $$QT_SOURCE_TREE/src/tools/qdatastream.h \ 
           $$QT_SOURCE_TREE/src/compat/qglist.h \
	   $$QT_SOURCE_TREE/src/tools/qlist.h \
	   $$QT_SOURCE_TREE/src/tools/qvector.h \
           $$QT_SOURCE_TREE/src/compat/qgarray.h \ 
           $$QT_SOURCE_TREE/src/tools/qglobal.h \ 
           $$QT_SOURCE_TREE/src/compat/qptrcollection.h \
           $$QT_SOURCE_TREE/src/tools/qbuffer.h \
           $$QT_SOURCE_TREE/src/tools/qstringlist.h \
           $$QT_SOURCE_TREE/src/compat/qcstring.h \
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
           $$QT_SOURCE_TREE/src/compat/qgcache.cpp \
           $$QT_SOURCE_TREE/src/tools/qbitarray.cpp \
	   $$QT_SOURCE_TREE/src/tools/qbytearray.cpp \
	   $$QT_SOURCE_TREE/src/tools/qchar.cpp \
           $$QT_SOURCE_TREE/src/compat/qgdict.cpp \
           $$QT_SOURCE_TREE/src/compat/qgvector.cpp \
           $$QT_SOURCE_TREE/src/tools/qdatastream.cpp \
           $$QT_SOURCE_TREE/src/compat/qglist.cpp \
	   $$QT_SOURCE_TREE/src/tools/qlist.cpp \
	   $$QT_SOURCE_TREE/src/tools/qvector.cpp \
           $$QT_SOURCE_TREE/src/compat/qgarray.cpp \
           $$QT_SOURCE_TREE/src/tools/qglobal.cpp \
           $$QT_SOURCE_TREE/src/compat/qptrcollection.cpp \
           $$QT_SOURCE_TREE/src/tools/qbuffer.cpp \
           $$QT_SOURCE_TREE/src/tools/qstringlist.cpp \
           $$QT_SOURCE_TREE/src/compat/qcstring.cpp \
           $$QT_SOURCE_TREE/src/tools/qtextstream.cpp \
           $$QT_SOURCE_TREE/src/tools/qdatetime.cpp \
           $$QT_SOURCE_TREE/src/codecs/qtextcodec.cpp \
           $$QT_SOURCE_TREE/src/tools/qmap.cpp \
           $$QT_SOURCE_TREE/src/tools/qlibrary.cpp \
           $$QT_SOURCE_TREE/src/tools/qlibrary_win.cpp \
           $$QT_SOURCE_TREE/src/tools/qunicodetables.cpp

INCLUDEPATH += $$QT_SOURCE_TREE/include/ $$QT_SOURCE_TREE/src/tools/
INTERFACES=
TARGET=configure
DESTDIR=../../dist/win/bin
LIBS = ole32.lib
win32-msvc.net : QMAKE_CXXFLAGS += /EHsc
win32-g++:LIBS += -luuid
win32:LIBS += -lole32

win32:CONFIG+=console

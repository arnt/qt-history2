CONFIG   = console release flat
DEFINES  = QT_NODLL QT_NO_CODECS QT_NO_TEXTCODEC QT_NO_UNICODETABLES QT_LITE_COMPONENT QT_NO_STL QT_NO_COMPRESS QT_BUILD_QMAKE QT_COMPAT QT_NO_THREAD
HEADERS	 = configureapp.h

SOURCES	 = main.cpp configureapp.cpp \
           $$QT_SOURCE_TREE/src/core/io/qdir.cpp \
           $$QT_SOURCE_TREE/src/core/io/qdir_win.cpp \
           $$QT_SOURCE_TREE/src/core/tools/qstring.cpp \
           $$QT_SOURCE_TREE/src/core/tools/qstringmatcher.cpp \
           $$QT_SOURCE_TREE/src/core/io/qfileinfo.cpp \
           $$QT_SOURCE_TREE/src/core/io/qfileinfo_win.cpp \
           $$QT_SOURCE_TREE/src/core/io/qfile.cpp \
           $$QT_SOURCE_TREE/src/core/io/qfile_win.cpp \
           $$QT_SOURCE_TREE/src/core/io/qiodevice.cpp \
           $$QT_SOURCE_TREE/src/core/tools/qregexp.cpp \
           $$QT_SOURCE_TREE/src/core/tools/qbitarray.cpp \
	   $$QT_SOURCE_TREE/src/core/tools/qbytearray.cpp \
	   $$QT_SOURCE_TREE/src/core/tools/qbytearraymatcher.cpp \
	   $$QT_SOURCE_TREE/src/core/tools/qchar.cpp \
           $$QT_SOURCE_TREE/src/core/io/qdatastream.cpp \
	   $$QT_SOURCE_TREE/src/core/tools/qlist.cpp \
	   $$QT_SOURCE_TREE/src/core/tools/qlocale.cpp \
	   $$QT_SOURCE_TREE/src/core/tools/qvector.cpp \
           $$QT_SOURCE_TREE/src/core/global/qglobal.cpp \
	   $$QT_SOURCE_TREE/src/core/tools/qhash.cpp \
           $$QT_SOURCE_TREE/src/core/io/qbuffer.cpp \
           $$QT_SOURCE_TREE/src/core/tools/qstringlist.cpp \
           $$QT_SOURCE_TREE/src/core/io/qtextstream.cpp \
           $$QT_SOURCE_TREE/src/core/tools/qdatetime.cpp \
           $$QT_SOURCE_TREE/src/core/codecs/qtextcodec.cpp \
           $$QT_SOURCE_TREE/src/core/tools/qmap.cpp \
           $$QT_SOURCE_TREE/src/core/library/qlibrary.cpp \
           $$QT_SOURCE_TREE/src/core/library/qlibrary_win.cpp \
           $$QT_SOURCE_TREE/src/core/tools/qunicodetables.cpp

INCLUDEPATH += $$QT_SOURCE_TREE/include/QtCore \
               $$QT_SOURCE_TREE/src/core/arch/generic
INTERFACES=
TARGET=configure
DESTDIR=../../bin
win32-msvc.net : QMAKE_CXXFLAGS += /EHsc
win32-g++:LIBS += -luuid
win32:LIBS += -lole32

win32:CONFIG+=console

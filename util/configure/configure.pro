CONFIG   = console release flat no_batch
DEFINES  = QT_NODLL QT_NO_CODECS QT_NO_TEXTCODEC QT_NO_UNICODETABLES QT_LITE_COMPONENT QT_NO_STL QT_NO_COMPRESS QT_BUILD_QMAKE QT_NO_THREAD QT_NO_QOBJECT

PRECOMPILED_HEADER = configure_pch.h

HEADERS	 = configureapp.h \
	   $$QT_SOURCE_TREE/src/core/tools/qbytearray.h \
	   $$QT_SOURCE_TREE/src/core/tools/qbytearraymatcher.h \
	   $$QT_SOURCE_TREE/src/core/tools/qchar.h \
	   $$QT_SOURCE_TREE/src/core/tools/qhash.h \
	   $$QT_SOURCE_TREE/src/core/tools/qlist.h \
	   $$QT_SOURCE_TREE/src/core/tools/qlocale.h \
	   $$QT_SOURCE_TREE/src/core/tools/qvector.h \
           $$QT_SOURCE_TREE/src/core/codecs/qtextcodec.h \
           $$QT_SOURCE_TREE/src/core/global/qglobal.h \
           $$QT_SOURCE_TREE/src/core/io/qbuffer.h \
           $$QT_SOURCE_TREE/src/core/io/qdatastream.h \
           $$QT_SOURCE_TREE/src/core/io/qdir.h \
           $$QT_SOURCE_TREE/src/core/io/qfile.h \
           $$QT_SOURCE_TREE/src/core/io/qfileinfo.h \
           $$QT_SOURCE_TREE/src/core/io/qfileengine.h \
           $$QT_SOURCE_TREE/src/core/io/qiodevice.h \
           $$QT_SOURCE_TREE/src/core/io/qtextstream.h \
           $$QT_SOURCE_TREE/src/core/io/qtemporaryfile.h \
           $$QT_SOURCE_TREE/src/core/tools/qbitarray.h \
           $$QT_SOURCE_TREE/src/core/tools/qdatetime.h \
           $$QT_SOURCE_TREE/src/core/tools/qmap.h \
           $$QT_SOURCE_TREE/src/core/tools/qregexp.h \
           $$QT_SOURCE_TREE/src/core/tools/qstring.h \
           $$QT_SOURCE_TREE/src/core/tools/qstringlist.h \
           $$QT_SOURCE_TREE/src/core/tools/qstringmatcher.h \
           $$QT_SOURCE_TREE/src/core/tools/qunicodetables_p.h 
 	   

SOURCES	 = main.cpp configureapp.cpp \
	   $$QT_SOURCE_TREE/src/core/tools/qbytearray.cpp \
	   $$QT_SOURCE_TREE/src/core/tools/qbytearraymatcher.cpp \
	   $$QT_SOURCE_TREE/src/core/tools/qchar.cpp \
	   $$QT_SOURCE_TREE/src/core/tools/qhash.cpp \
	   $$QT_SOURCE_TREE/src/core/tools/qlist.cpp \
	   $$QT_SOURCE_TREE/src/core/tools/qlocale.cpp \
	   $$QT_SOURCE_TREE/src/core/tools/qvector.cpp \
           $$QT_SOURCE_TREE/src/core/codecs/qtextcodec.cpp \
           $$QT_SOURCE_TREE/src/core/global/qglobal.cpp \
           $$QT_SOURCE_TREE/src/core/io/qbuffer.cpp \
           $$QT_SOURCE_TREE/src/core/io/qdatastream.cpp \
           $$QT_SOURCE_TREE/src/core/io/qdir.cpp \
           $$QT_SOURCE_TREE/src/core/io/qfile.cpp \
           $$QT_SOURCE_TREE/src/core/io/qfileinfo.cpp \
           $$QT_SOURCE_TREE/src/core/io/qfileengine.cpp \
           $$QT_SOURCE_TREE/src/core/io/qfileengine_win.cpp \
           $$QT_SOURCE_TREE/src/core/io/qiodevice.cpp \
           $$QT_SOURCE_TREE/src/core/io/qtextstream.cpp \
           $$QT_SOURCE_TREE/src/core/io/qtemporaryfile.cpp \
           $$QT_SOURCE_TREE/src/core/tools/qbitarray.cpp \
           $$QT_SOURCE_TREE/src/core/tools/qdatetime.cpp \
           $$QT_SOURCE_TREE/src/core/tools/qmap.cpp \
           $$QT_SOURCE_TREE/src/core/tools/qregexp.cpp \
           $$QT_SOURCE_TREE/src/core/tools/qstring.cpp \
           $$QT_SOURCE_TREE/src/core/tools/qstringlist.cpp \
           $$QT_SOURCE_TREE/src/core/tools/qstringmatcher.cpp \
           $$QT_SOURCE_TREE/src/core/tools/qunicodetables.cpp \
           $$QT_SOURCE_TREE/src/core/tools/qvsnprintf.cpp

SOURCES += $$QT_SOURCE_TREE/util/install/keygen/keyinfo.cpp

INCLUDEPATH += $$QT_SOURCE_TREE/src/core/arch/generic \
	       $$QT_SOURCE_TREE/include/QtCore \
	       $$QT_SOURCE_TREE/util/install/keygen


INTERFACES=
TARGET=configure
DESTDIR=../../bin
win32-msvc.net : QMAKE_CXXFLAGS += /EHsc
win32-g++:LIBS += -luuid
win32:LIBS += -lole32 -ladvapi32
win32:CONFIG+=console

win32-msvc* {
    QMAKE_CFLAGS_RELEASE -= -MD
    QMAKE_CFLAGS_DEBUG -= -MDd
    QMAKE_CXXFLAGS_RELEASE -= -MD
    QMAKE_CXXFLAGS_DEBUG -= -MDd

    QMAKE_CFLAGS_RELEASE += -ML
    QMAKE_CFLAGS_DEBUG += -MLd
    QMAKE_CXXFLAGS_RELEASE += -ML
    QMAKE_CXXFLAGS_DEBUG += -MLd
}

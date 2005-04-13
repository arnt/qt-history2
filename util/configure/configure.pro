CONFIG   = console release flat no_batch
DEFINES  = QT_NODLL QT_NO_CODECS QT_NO_TEXTCODEC QT_NO_UNICODETABLES QT_LITE_COMPONENT QT_NO_STL QT_NO_COMPRESS QT_BUILD_QMAKE QT_NO_THREAD QT_NO_QOBJECT

PRECOMPILED_HEADER = configure_pch.h

INCPATH += $$QT_SOURCE_TREE/src/corelib/arch/generic \
           $$QT_SOURCE_TREE/include \
           $$QT_SOURCE_TREE/include/QtCore \

HEADERS	 = configureapp.h \
	   $$QT_SOURCE_TREE/src/corelib/tools/qbytearray.h \
	   $$QT_SOURCE_TREE/src/corelib/tools/qbytearraymatcher.h \
	   $$QT_SOURCE_TREE/src/corelib/tools/qchar.h \
	   $$QT_SOURCE_TREE/src/corelib/tools/qhash.h \
	   $$QT_SOURCE_TREE/src/corelib/tools/qlist.h \
	   $$QT_SOURCE_TREE/src/corelib/tools/qlocale.h \
	   $$QT_SOURCE_TREE/src/corelib/tools/qvector.h \
           $$QT_SOURCE_TREE/src/corelib/codecs/qtextcodec.h \
           $$QT_SOURCE_TREE/src/corelib/global/qglobal.h \
           $$QT_SOURCE_TREE/src/corelib/io/qbuffer.h \
	   $$QT_SOURCE_TREE/src/corelib/io/qbufferedfsfileengine.h \
           $$QT_SOURCE_TREE/src/corelib/io/qdatastream.h \
           $$QT_SOURCE_TREE/src/corelib/io/qdir.h \
           $$QT_SOURCE_TREE/src/corelib/io/qfile.h \
           $$QT_SOURCE_TREE/src/corelib/io/qfileinfo.h \
           $$QT_SOURCE_TREE/src/corelib/io/qfsfileengine.h \
           $$QT_SOURCE_TREE/src/corelib/io/qiodevice.h \
           $$QT_SOURCE_TREE/src/corelib/io/qtextstream.h \
           $$QT_SOURCE_TREE/src/corelib/io/qtemporaryfile.h \
           $$QT_SOURCE_TREE/src/corelib/tools/qbitarray.h \
           $$QT_SOURCE_TREE/src/corelib/tools/qdatetime.h \
           $$QT_SOURCE_TREE/src/corelib/tools/qmap.h \
           $$QT_SOURCE_TREE/src/corelib/tools/qregexp.h \
           $$QT_SOURCE_TREE/src/corelib/tools/qstring.h \
           $$QT_SOURCE_TREE/src/corelib/tools/qstringlist.h \
           $$QT_SOURCE_TREE/src/corelib/tools/qstringmatcher.h \
           $$QT_SOURCE_TREE/src/corelib/tools/qunicodetables_p.h 
 	   

SOURCES	 = main.cpp configureapp.cpp \
	   $$QT_SOURCE_TREE/src/corelib/tools/qbytearray.cpp \
	   $$QT_SOURCE_TREE/src/corelib/tools/qbytearraymatcher.cpp \
	   $$QT_SOURCE_TREE/src/corelib/tools/qchar.cpp \
	   $$QT_SOURCE_TREE/src/corelib/tools/qhash.cpp \
	   $$QT_SOURCE_TREE/src/corelib/tools/qlistdata.cpp \
	   $$QT_SOURCE_TREE/src/corelib/tools/qlocale.cpp \
	   $$QT_SOURCE_TREE/src/corelib/tools/qvector.cpp \
           $$QT_SOURCE_TREE/src/corelib/codecs/qtextcodec.cpp \
           $$QT_SOURCE_TREE/src/corelib/global/qglobal.cpp \
           $$QT_SOURCE_TREE/src/corelib/io/qbuffer.cpp \
	   $$QT_SOURCE_TREE/src/corelib/io/qbufferedfsfileengine.cpp \
           $$QT_SOURCE_TREE/src/corelib/io/qdatastream.cpp \
           $$QT_SOURCE_TREE/src/corelib/io/qdir.cpp \
           $$QT_SOURCE_TREE/src/corelib/io/qfile.cpp \
           $$QT_SOURCE_TREE/src/corelib/io/qfileinfo.cpp \
           $$QT_SOURCE_TREE/src/corelib/io/qfileengine.cpp \
           $$QT_SOURCE_TREE/src/corelib/io/qfsfileengine.cpp \
           $$QT_SOURCE_TREE/src/corelib/io/qfsfileengine_win.cpp \
           $$QT_SOURCE_TREE/src/corelib/io/qiodevice.cpp \
           $$QT_SOURCE_TREE/src/corelib/io/qtextstream.cpp \
           $$QT_SOURCE_TREE/src/corelib/io/qtemporaryfile.cpp \
           $$QT_SOURCE_TREE/src/corelib/tools/qbitarray.cpp \
           $$QT_SOURCE_TREE/src/corelib/tools/qdatetime.cpp \
           $$QT_SOURCE_TREE/src/corelib/tools/qmap.cpp \
           $$QT_SOURCE_TREE/src/corelib/tools/qregexp.cpp \
           $$QT_SOURCE_TREE/src/corelib/tools/qstring.cpp \
           $$QT_SOURCE_TREE/src/corelib/tools/qstringlist.cpp \
           $$QT_SOURCE_TREE/src/corelib/tools/qstringmatcher.cpp \
           $$QT_SOURCE_TREE/src/corelib/tools/qunicodetables.cpp \
           $$QT_SOURCE_TREE/src/corelib/tools/qvsnprintf.cpp

SOURCES += $$QT_SOURCE_TREE/util/install/keygen/keyinfo.cpp

INCLUDEPATH += $$QT_SOURCE_TREE/src/corelib/arch/generic \
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

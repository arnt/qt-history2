CONFIG   = console release
DEFINES  = QT_NODLL QT_NO_CODECS QT_NO_TEXTCODEC QT_NO_UNICODETABLES QT_LITE_COMPONENT QT_NO_STL QT_NO_COMPRESS QT_BUILD_QMAKE QT_COMPAT
HEADERS	 = configureapp.h

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
           $$QT_SOURCE_TREE/src/tools/qbitarray.cpp \
	   $$QT_SOURCE_TREE/src/tools/qbytearray.cpp \
	   $$QT_SOURCE_TREE/src/tools/qchar.cpp \
           $$QT_SOURCE_TREE/src/tools/qdatastream.cpp \
           $$QT_SOURCE_TREE/src/compat/qglist.cpp \
	   $$QT_SOURCE_TREE/src/tools/qlist.cpp \
	   $$QT_SOURCE_TREE/src/tools/qlocale.cpp \
	   $$QT_SOURCE_TREE/src/compat/qgvector.cpp \
	   $$QT_SOURCE_TREE/src/tools/qvector.cpp \
           $$QT_SOURCE_TREE/src/tools/qglobal.cpp \
	   $$QT_SOURCE_TREE/src/tools/qhash.cpp \
           $$QT_SOURCE_TREE/src/compat/qptrcollection.cpp \
           $$QT_SOURCE_TREE/src/tools/qbuffer.cpp \
           $$QT_SOURCE_TREE/src/tools/qstringlist.cpp \
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
win32-msvc.net : QMAKE_CXXFLAGS += /EHsc
win32-g++:LIBS += -luuid
win32:LIBS += -lole32

win32:CONFIG+=console

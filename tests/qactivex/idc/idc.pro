TEMPLATE 	= app
CONFIG  	= console release warn_off qtinc
DEFINES 	= QT_NO_CODECS QT_LITE_UNICODE QT_NO_COMPONENT QT_NO_STL QT_NODLL
INCLUDEPATH 	= $$QT_SOURCE_TREE/include .
DEPENDPATH	= $$QT_SOURCE_TREE/include .

SOURCES		= main.cpp \
		    $$QT_SOURCE_TREE/src/tools/qbuffer.cpp \
		    $$QT_SOURCE_TREE/src/tools/qptrcollection.cpp \
		    $$QT_SOURCE_TREE/src/tools/qcstring.cpp \
		    $$QT_SOURCE_TREE/src/tools/qdatastream.cpp \
		    $$QT_SOURCE_TREE/src/tools/qdatetime.cpp \
		    $$QT_SOURCE_TREE/src/tools/qdir.cpp \
		    $$QT_SOURCE_TREE/src/tools/qfile.cpp \
		    $$QT_SOURCE_TREE/src/tools/qfileinfo.cpp \
		    $$QT_SOURCE_TREE/src/tools/qgarray.cpp \
		    $$QT_SOURCE_TREE/src/tools/qgdict.cpp       \
		    $$QT_SOURCE_TREE/src/tools/qglist.cpp       \
		    $$QT_SOURCE_TREE/src/tools/qglobal.cpp      \
		    $$QT_SOURCE_TREE/src/tools/qgvector.cpp     \
		    $$QT_SOURCE_TREE/src/tools/qiodevice.cpp    \
		    $$QT_SOURCE_TREE/src/tools/qlibrary.cpp    \
		    $$QT_SOURCE_TREE/src/tools/qregexp.cpp      \
		    $$QT_SOURCE_TREE/src/tools/qstring.cpp      \
		    $$QT_SOURCE_TREE/src/tools/qstringlist.cpp  \
		    $$QT_SOURCE_TREE/src/tools/qtextstream.cpp  \
		    $$QT_SOURCE_TREE/src/tools/qbitarray.cpp    \
		    $$QT_SOURCE_TREE/src/tools/qmap.cpp         \
		    $$QT_SOURCE_TREE/src/tools/qgcache.cpp      \
		    $$QT_SOURCE_TREE/src/codecs/qtextcodec.cpp \
		    $$QT_SOURCE_TREE/src/codecs/qutfcodec.cpp

unix:SOURCES    += $$QT_SOURCE_TREE/src/tools/qdir_unix.cpp \
		   $$QT_SOURCE_TREE/src/tools/qfile_unix.cpp \
		   $$QT_SOURCE_TREE/src/tools/qfileinfo_unix.cpp \
		   $$QT_SOURCE_TREE/src/tools/qlibrary_unix.cpp
win32:SOURCES   += $$QT_SOURCE_TREE/src/tools/qdir_win.cpp \
		   $$QT_SOURCE_TREE/src/tools/qfile_win.cpp \
		   $$QT_SOURCE_TREE/src/tools/qfileinfo_win.cpp \
		   $$QT_SOURCE_TREE/src/tools/qlibrary_win.cpp

win32:LIBS	+= ole32.lib

TARGET 		= idc
DESTDIR 	= $$QT_BUILD_TREE\bin

TEMPLATE 	= app
CONFIG  	= console release warn_off qtinc
DEFINES 	= QT_NO_CODECS QT_LITE_UNICODE QT_NO_COMPONENT QT_NO_STL QT_NODLL
INCLUDEPATH 	= $(QTDIR)/include .
DEPENDPATH	= $(QTDIR)/include .

SOURCES		= main.cpp \
		    $(QTDIR)/src/tools/qbuffer.cpp \
		    $(QTDIR)/src/tools/qptrcollection.cpp \
		    $(QTDIR)/src/tools/qcstring.cpp \
		    $(QTDIR)/src/tools/qdatastream.cpp \
		    $(QTDIR)/src/tools/qdatetime.cpp \
		    $(QTDIR)/src/tools/qdir.cpp \
		    $(QTDIR)/src/tools/qfile.cpp \
		    $(QTDIR)/src/tools/qfileinfo.cpp \
		    $(QTDIR)/src/tools/qgarray.cpp \
		    $(QTDIR)/src/tools/qgdict.cpp       \
		    $(QTDIR)/src/tools/qglist.cpp       \
		    $(QTDIR)/src/tools/qglobal.cpp      \
		    $(QTDIR)/src/tools/qgvector.cpp     \
		    $(QTDIR)/src/tools/qiodevice.cpp    \
		    $(QTDIR)/src/tools/qlibrary.cpp    \
		    $(QTDIR)/src/tools/qregexp.cpp      \
		    $(QTDIR)/src/tools/qstring.cpp      \
		    $(QTDIR)/src/tools/qstringlist.cpp  \
		    $(QTDIR)/src/tools/qtextstream.cpp  \
		    $(QTDIR)/src/tools/qbitarray.cpp    \
		    $(QTDIR)/src/tools/qmap.cpp         \
		    $(QTDIR)/src/tools/qgcache.cpp      \
		    $(QTDIR)/src/codecs/qtextcodec.cpp \
		    $(QTDIR)/src/codecs/qutfcodec.cpp

unix:SOURCES    += $(QTDIR)/src/tools/qdir_unix.cpp \
		   $(QTDIR)/src/tools/qfile_unix.cpp \
		   $(QTDIR)/src/tools/qfileinfo_unix.cpp \
		   $(QTDIR)/src/tools/qlibrary_unix.cpp
win32:SOURCES   += $(QTDIR)/src/tools/qdir_win.cpp \
		   $(QTDIR)/src/tools/qfile_win.cpp \
		   $(QTDIR)/src/tools/qfileinfo_win.cpp \
		   $(QTDIR)/src/tools/qlibrary_win.cpp

win32:LIBS	+= ole32.lib

TARGET 		= idc
DESTDIR 	= $(QTDIR)\bin

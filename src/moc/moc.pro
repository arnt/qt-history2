TEMPLATE	= app
CONFIG		= console release qtinc yacc
DEFINES		= QT_NO_CODECS QT_LITE_UNICODE
win32:DEFINES  += QT_NODLL
LEXSOURCES	= moc.l
YACCSOURCES	= moc.y
INCLUDEPATH	= ../../include ../tools .
DEPENDPATH	= ../../include ../tools .
DESTDIR         = $(QTDIR)/bin
OBJECTS_DIR	= .
SOURCES		= ../tools/qbuffer.cpp	    \
		  ../tools/qcollection.cpp  \
		  ../tools/qcstring.cpp	    \
		  ../tools/qdatastream.cpp  \
		  ../tools/qdatetime.cpp    \
		  ../tools/qfile.cpp	    \
		  ../tools/qgarray.cpp	    \
		  ../tools/qgdict.cpp	    \
		  ../tools/qglist.cpp	    \
		  ../tools/qglobal.cpp	    \
		  ../tools/qgvector.cpp	    \
		  ../tools/qiodevice.cpp    \
		  ../tools/qregexp.cpp	    \
		  ../tools/qstring.cpp	    \
		  ../tools/qstringlist.cpp  \
		  ../tools/qtextstream.cpp  \
		  ../tools/qbitarray.cpp    \
		  ../tools/qmap.cpp         \
		  ../tools/qgcache.cpp      \
		  ../codecs/qtextcodec.cpp

unix:SOURCES	+= ../tools/qfile_unix.cpp

win32:SOURCES	+= ../tools/qfile_win.cpp

TARGET		= moc

isEmpty(INSTALLtarget_PATH):INSTALLtarget_PATH=/home/sam/blah/qt/bin
INSTALLS       += target

TEMPLATE	= app
CONFIG = console release qtinc yacc lex_included yacc_no_name_mangle
DEFINES	       += QT_MOC QT_NO_CODECS QT_LITE_UNICODE QT_NO_COMPONENT QT_NO_STL
win32:DEFINES  += QT_NODLL
LEXSOURCES	= moc.l
YACCSOURCES	= moc.y
INCLUDEPATH	= $(QTDIR)/include ../tools .
DEPENDPATH	= $(QTDIR)/include ../tools .
LIBS		=
DESTDIR         = ../../bin
OBJECTS_DIR	= .
SOURCES		= ../tools/qbuffer.cpp	    \
		  ../tools/qptrcollection.cpp  \
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
		  ../codecs/qtextcodec.cpp \
		  ../codecs/qutfcodec.cpp

unix:SOURCES	+= ../tools/qfile_unix.cpp
win32:SOURCES	+= ../tools/qfile_win.cpp
macx:LIBS += -framework Carbon

TARGET		= moc

target.path=$$bin.path
isEmpty(target.path):target.path=$$QT_PREFIX/bin
INSTALLS       += target

*-mwerks {
   TEMPLATE = lib
   TARGET = McMoc
   CONFIG -= static
   CONFIG += shared plugin
   DEFINES += MOC_MWERKS_PLUGIN
   MWERKSDIR = $(QTDIR)/util/mwerks_plugin
   INCLUDEPATH += $$MWERKSDIR/Headers
   LIBS += $$MWERKSDIR/Libraries/PluginLib4.shlb
   SOURCES += mwerks_mac.cpp
}

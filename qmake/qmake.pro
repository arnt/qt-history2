#This is a project file for building qmake, of course it presents a problem -
# it is very hard to make qmake build this, when qmake is the thing it builds,
#once you are boot strapped though, the qmake.pro will offer better coverage of a
#platform than either of the generic makefiles
SKIP_DEPENDS += qconfig.h qmodules.h
CONFIG += console
CONFIG -= opengl qt shared
DESTIR = ../bin/
DEPENDPATH += generators generators/unix generators/win32 \
              generators/mac $(QTDIR)/include $(QTDIR)/qmake
INCLUDEPATH += $$DEPENDPATH
DEFINES += QT_NO_TEXTCODEC QT_LITE_COMPONENT QT_NO_STL

#qmake code
SOURCES+=project.cpp main.cpp makefile.cpp \
         unixmake2.cpp unixmake.cpp \
         borland_bmake.cpp msvc_nmake.cpp \
	 msvc_dsp.cpp option.cpp winmakefile.cpp \
	 projectgenerator.cpp metrowerks_xml.cpp \
         pbuilder_pbx.cpp

#Qt code
SOURCES+=qstring.cpp qtextstream.cpp \
         qiodevice.cpp qglobal.cpp \
	 qgdict.cpp qcstring.cpp \
	 qdatastream.cpp qgarray.cpp \
	 qbuffer.cpp qglist.cpp \
	 qptrcollection.cpp qfile.cpp \
	 qregexp.cpp \
	 qgvector.cpp qgcache.cpp \
	 qbitarray.cpp qdir.cpp \
	 qfileinfo.cpp qdatetime.cpp \
	 qstringlist.cpp qmap.cpp
#where to find the Qt code, and platform dependant SOURCES
unix {
   VPATH = $$QT_SOURCE_TREE/src/tools
   SOURCES += qfile_unix.cpp qfileinfo_unix.cpp qdir_unix.cpp 
}
win32 {
   VPATH = $(QTDIR)/src/tools
   SOURCES += qfile_win.cpp qfileinfo_win.cpp qdir_win.cpp 
   *-msvc:LIBS += ole32.lib
}
macx-*: LIBS += -framework Carbon

#installation
target.path=$$bin.path
isEmpty(target.path):target.path=$$QT_PREFIX/bin
INSTALLS        += target


isEmpty(data.path):data.path=$$QT_PREFIX
mkspecs.path=$$data.path/mkspecs
mkspecs.files=$(QTDIR)/mkspecs
INSTALLS        += mkspecs

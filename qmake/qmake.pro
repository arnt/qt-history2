#This is a project file for building qmake, of course it presents a problem -
# it is very hard to make qmake build this, when qmake is the thing it builds,
#once you are boot strapped though, the qmake.pro will offer better coverage of a
#platform than either of the generic makefiles
CONFIG += console
CONFIG -= opengl
DESTIR = ../bin/

#qmake code
SOURCES+=project.cpp main.cpp makefile.cpp \
         unixmake2.cpp unixmake.cpp \
         borland_bmake.cpp msvc_nmake.cpp \
	 msvc_dsp.cpp option.cpp winmakefile.cpp \
	 projectgenerator.cpp metrowerks_xml.cpp \
         pbuilder_pbx.cpp

#qt code
SOURCES+=qstring.cpp qtextstream.cpp \
         qiodevice.cpp qglobal.cpp \
	 qgdict.cpp qcstring.cpp \
	 qdatastream.cpp qgarray.cpp \
	 qbuffer.cpp qglist.cpp \
	 qptrcollection.cpp qfile.cpp \
	 qfile_unix.cpp qregexp.cpp \
	 qgvector.cpp qgcache.cpp \
	 qbitarray.cpp qdir.cpp \
	 qfileinfo_unix.cpp qdir_unix.cpp \
	 qfileinfo.cpp qdatetime.cpp \
	 qstringlist.cpp qmap.cpp

DEPENDPATH += generators generators/unix generators/win32 \
              generators/mac $(QTDIR)/include $(QTDIR)/qmake
INCLUDEPATH += $$DEPENDPATH
VPATH = $$DEPENDPATH $$QT_SOURCE_TREE/src/tools

DEFINES += QT_NO_TEXTCODEC QT_LITE_COMPONENT QT_NO_STL

target.path=$$QT_INSTALL_BINPATH
isEmpty(target.path):target.path=$$QT_PREFIX/bin
INSTALLS        += target

mkspecs.path=$$QT_PREFIX/mkspecs
mkspecs.files=$(QTDIR)/mkspecs
INSTALLS        += mkspecs

macx-* {
     INCLUDEPATH += /System/Library/Frameworks/CoreServices.framework/Frameworks/CarbonCore.framework/Headers/
     LIBS += -framework Carbon
}

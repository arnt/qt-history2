#This is a project file for building qmake, of course it presents a problem -
# it is very hard to make qmake build this, when qmake is the thing it builds,
#once you are boot strapped though, the qmake.pro will offer better coverage of a
#platform than either of the generic makefiles
QMAKE_INCREMENTAL =
SKIP_DEPENDS += qconfig.h qmodules.h
CONFIG += console
CONFIG -= qt shared resource_fork
DESTDIR = ../bin/
DEPENDPATH += generators generators/unix generators/win32 \
              generators/mac $$QT_SOURCE_TREE/include $$QT_SOURCE_TREE/include/QtCore $$QT_SOURCE_TREE/qmake
INCLUDEPATH += $$DEPENDPATH .
DEFINES += QT_NO_TEXTCODEC QT_NO_COMPONENT QT_NO_STL QT_NO_COMPRESS QT_NO_UNICODETABLES QT_NODLL QT_BUILD_QMAKE QT_NO_THREAD

OBJECTS_DIR = .
MOC_DIR = .

#qmake code
SOURCES+=project.cpp property.cpp main.cpp makefile.cpp \
         unixmake2.cpp unixmake.cpp meta.cpp \
         borland_bmake.cpp msvc_nmake.cpp \
	 msvc_dsp.cpp msvc_vcproj.cpp option.cpp winmakefile.cpp \
	 projectgenerator.cpp metrowerks_xml.cpp mingw_make.cpp \
         pbuilder_pbx.cpp msvc_objectmodel.cpp qtmd5.cpp makefiledeps.cpp \
         metamakefile.cpp xmloutput.cpp
HEADERS+=project.h property.h makefile.h \
         unixmake.h meta.h \
         borland_bmake.h msvc_nmake.h \
	 msvc_dsp.h msvc_vcproj.h option.h winmakefile.h \
	 projectgenerator.h metrowerks_xml.h mingw_make.h \
         pbuilder_pbx.h msvc_objectmodel.h qtmd5.h makefiledeps.h \
         metamakefile.h xmloutput.h

#Qt code
SOURCES+=qchar.cpp qstring.cpp qstringmatcher.cpp \
         qtextstream.cpp qiodevice.cpp qglobal.cpp \
	 qbytearray.cpp qbytearraymatcher.cpp \
	 qdatastream.cpp qbuffer.cpp qlist.cpp\
	 qfile.cpp qregexp.cpp quuid.cpp \
	 qvector.cpp qbitarray.cpp qdir.cpp qhash.cpp \
	 qfileinfo.cpp qdatetime.cpp qstringlist.cpp qmap.cpp \
	 qsettings.cpp qunicodetables.cpp qlocale.cpp \
	 qfsfileengine.cpp qfsfileinfoengine.cpp qfsdirengine.cpp
HEADERS+=qchar.h qstring.h qstringmatcher.h \
         qtextstream.h qiodevice.h qglobal.h \
	 qbytearray.h qbytearraymatcher.h \
	 qdatastream.h qbuffer.h qlist.h\
	 qfile.h qregexp.h quuid.h \
	 qvector.h qbitarray.h qdir.h qhash.h \
	 qfileinfo.h qdatetime.h qstringlist.h qmap.h \
	 qsettings.h qlocale.h \
	 qfileengine.h qfileinfoengine.h qdirengine.h \
	 qfileengine_p.h qfileinfoengine_p.h qdirengine_p.h 

exists($$QT_BUILD_TREE/src/core/global/qconfig.cpp) {  #qconfig.cpp
    DEFINES += HAVE_QCONFIG_CPP
    SOURCES += $$QT_BUILD_TREE/src/core/global/qconfig.cpp
}

#where to find the Qt code, and platform dependant SOURCES
VPATH += $$QT_SOURCE_TREE/src/core/global \
         $$QT_SOURCE_TREE/src/core/tools \
         $$QT_SOURCE_TREE/src/core/kernel \
         $$QT_SOURCE_TREE/src/core/library \
	 $$QT_SOURCE_TREE/src/core/io 

unix {
   SOURCES += qfsfileengine_unix.cpp qfsfileinfoengine_unix.cpp qfsdirengine_unix.cpp 
   mac:SOURCES += qsettings_mac.cpp qurl.cpp qcore_mac.cpp
}

win32 {
   SOURCES += qfsfileengine_win.cpp qfsfileinfoengine_win.cpp qfsdirengine_win.cpp \
              qsettings_win.cpp qlibrary_win.cpp qlibrary.cpp 
   HEADERS += qsettings.h qlibrary.h qlibrary.h 
   win32-msvc*:LIBS += ole32.lib advapi32.lib
}
macx-*: LIBS += -framework CoreServices

qnx {
    CFLAGS += -fhonor-std
    LFLAGS += -lcpp
}
#installation
target.path=$$bins.path
INSTALLS        += target



mkspecs.path=$$data.path/mkspecs
mkspecs.files=$$QT_SOURCE_TREE/mkspecs
INSTALLS        += mkspecs

*-g++:profiling {
  QMAKE_CFLAGS = -pg
  QMAKE_CXXFLAGS = -pg
  QMAKE_LFLAGS = -pg
}

PRECOMPILED_HEADER = qmake_pch.h

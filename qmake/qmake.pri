CONFIG += depend_includepath

QMAKE_INCREMENTAL =
SKIP_DEPENDS += qconfig.h qmodules.h
DEFINES += QT_NO_TEXTCODEC QT_NO_COMPONENT QT_NO_STL QT_NO_COMPRESS QT_NO_UNICODETABLES

#qmake code
SOURCES+= project.cpp property.cpp main.cpp generators/makefile.cpp \
          generators/unix/unixmake2.cpp generators/unix/unixmake.cpp meta.cpp \
          generators/win32/borland_bmake.cpp generators/win32/msvc_nmake.cpp \
	  generators/win32/msvc_dsp.cpp generators/win32/msvc_vcproj.cpp \
          option.cpp generators/win32/winmakefile.cpp generators/win32/mingw_make.cpp \
	  generators/projectgenerator.cpp generators/mac/metrowerks_xml.cpp \
          generators/mac/pbuilder_pbx.cpp generators/win32/msvc_objectmodel.cpp \
	  generators/makefiledeps.cpp generators/metamakefile.cpp \
	  qtmd5.cpp generators/xmloutput.cpp
HEADERS+= project.h property.h generators/makefile.h \
          generators/unix/unixmake.h meta.h \
          generators/win32/borland_bmake.h generators/win32/msvc_nmake.h \
	  generators/win32/msvc_dsp.h generators/win32/msvc_vcproj.h option.h \
	  generators/win32/winmakefile.h generators/projectgenerator.h \
	  generators/mac/metrowerks_xml.h generators/win32/mingw_make.h \
	  generators/mac/pbuilder_pbx.h generators/win32/msvc_objectmodel.h \
	  qtmd5.h generators/makefiledeps.h generators/metamakefile.h \
	  generators/xmloutput.h

bootstrap { #Qt code
   DEFINES+=QT_NODLL QT_NO_THREAD
   SOURCES+=qchar.cpp qstring.cpp qstringmatcher.cpp \
            qtextstream.cpp qiodevice.cpp qglobal.cpp \
	    qbytearray.cpp qbytearraymatcher.cpp \
	    qdatastream.cpp qbuffer.cpp qlist.cpp\
	    qfile.cpp qregexp.cpp quuid.cpp qioengine.cpp \
	    qvector.cpp qbitarray.cpp qdir.cpp qhash.cpp \
	    qfileinfo.cpp qdatetime.cpp qstringlist.cpp qmap.cpp \
	    qcoresettings.cpp qunicodetables.cpp qlocale.cpp qfileengine.cpp \
	    qtemporaryfile.cpp
   HEADERS+=qchar.h qstring.h qstringmatcher.h \
            qtextstream.h qiodevice.h qglobal.h \
	    qbytearray.h qbytearraymatcher.h \
	    qdatastream.h qbuffer.h qlist.h\
	    qfile.h qregexp.h quuid.h \
	    qvector.h qbitarray.h qdir.h qhash.h \
	    qfileinfo.h qdatetime.h qstringlist.h qmap.h \
	    qcoresettings.h qlocale.h qfileengine.h qioengine.h qtemporaryfile.h

    exists($$QT_BUILD_TREE/src/core/global/qconfig.cpp) {  #qconfig.cpp
       DEFINES += HAVE_QCONFIG_CPP
       SOURCES += $$QT_BUILD_TREE/src/core/global/qconfig.cpp
    }

    unix {
        SOURCES += qfileengine_unix.cpp
        mac:SOURCES += qcoresettings_mac.cpp qurl.cpp qcore_mac.cpp
    } else:win32 {
       SOURCES += qfileengine_win.cpp qcoresettings_win.cpp
       win32-msvc*:LIBS += ole32.lib advapi32.lib
    }
    macx-*: LIBS += -framework CoreServices

    qnx {
        CFLAGS += -fhonor-std
        LFLAGS += -lcpp
    }
} else {
    CONFIG += qt
    QT = core
}
*-g++:profiling {
  QMAKE_CFLAGS = -pg
  QMAKE_CXXFLAGS = -pg
  QMAKE_LFLAGS = -pg
}

PRECOMPILED_HEADER = qmake_pch.h

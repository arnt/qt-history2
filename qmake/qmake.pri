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
   SOURCES+= \
        qbitarray.cpp \ 
        qbuffer.cpp \
        qbytearray.cpp \
        qbytearraymatcher.cpp \
        qchar.cpp \
        qcorevariant.cpp \
        qdatastream.cpp \
        qdatetime.cpp \
        qdir.cpp \
        qfile.cpp \
        qfileengine.cpp \
        qfileinfo.cpp \
        qglobal.cpp \
        qhash.cpp \
        qiodevice.cpp \
        qlist.cpp \
        qlocale.cpp \
        qmap.cpp \
        qmetatype.cpp \
        qregexp.cpp \
        qstring.cpp \
        qstringlist.cpp \
        qstringmatcher.cpp \
        qtemporaryfile.cpp \
        qtextstream.cpp \
        qunicodetables.cpp \
        quuid.cpp \
        qvector.cpp 

   HEADERS+= \
        qbitarray.h \
        qbuffer.h \
        qbytearray.h \
        qbytearraymatcher.h \
        qchar.h \
        qcorevariant.h \
        qdatastream.h \
        qdatetime.h \
        qdir.h \
        qfile.h \
        qfileengine.h \
        qfileinfo.h \
        qglobal.h \
        qhash.h \
        qiodevice.h \
        qlist.h \
        qlocale.h \
        qmap.h \
        qmetatype.h \
        qregexp.h \
        qstring.h \
        qstringlist.h \
        qstringmatcher.h \
        qtemporaryfile.h \
        qtextstream.h \
        quuid.h \
        qvector.h
 
    exists($$QT_BUILD_TREE/src/core/global/qconfig.cpp) {  #qconfig.cpp
       DEFINES += HAVE_QCONFIG_CPP
       SOURCES += $$QT_BUILD_TREE/src/core/global/qconfig.cpp
    }

    unix {
        SOURCES += qfileengine_unix.cpp
        mac:SOURCES += qurl.cpp qcore_mac.cpp
    } else:win32 {
       SOURCES += qfileengine_win.cpp
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

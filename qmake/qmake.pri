CONFIG += depend_includepath

QMAKE_INCREMENTAL =
SKIP_DEPENDS += qconfig.h qmodules.h
DEFINES += QT_NO_TEXTCODEC QT_NO_COMPONENT QT_NO_STL QT_NO_COMPRESS QT_NO_UNICODETABLES

#qmake code
SOURCES += project.cpp property.cpp main.cpp generators/makefile.cpp \
           generators/unix/unixmake2.cpp generators/unix/unixmake.cpp meta.cpp \
           option.cpp generators/win32/winmakefile.cpp generators/win32/mingw_make.cpp \
           generators/makefiledeps.cpp generators/metamakefile.cpp \
           qtmd5.cpp generators/xmloutput.cpp
HEADERS += project.h property.h generators/makefile.h \
           generators/unix/unixmake.h meta.h option.h \
           generators/win32/winmakefile.h generators/projectgenerator.h \
           qtmd5.h generators/makefiledeps.h generators/metamakefile.h \
           generators/xmloutput.h
!contains(QT_PRODUCT, qt-free) {
   SOURCES +=  generators/win32/borland_bmake.cpp generators/win32/msvc_nmake.cpp \
               generators/projectgenerator.cpp generators/mac/metrowerks_xml.cpp \
               generators/win32/msvc_dsp.cpp generators/win32/msvc_vcproj.cpp \
               generators/mac/pbuilder_pbx.cpp generators/win32/msvc_objectmodel.cpp
   HEADERS +=  generators/win32/borland_bmake.h generators/win32/msvc_nmake.h \
               generators/win32/msvc_dsp.h generators/win32/msvc_vcproj.h \
               generators/mac/metrowerks_xml.h generators/win32/mingw_make.h \
               generators/mac/pbuilder_pbx.h generators/win32/msvc_objectmodel.h
} else {
   DEFINES += QMAKE_OPENSOURCE_EDITION
}

bootstrap { #Qt code
   DEFINES+=QT_NODLL QT_NO_THREAD
   SOURCES+= \
        qbitarray.cpp \
        qbuffer.cpp \
	qbufferedfsfileengine.cpp \
        qbytearray.cpp \
        qbytearraymatcher.cpp \
        qchar.cpp \
        qdatastream.cpp \
        qdatetime.cpp \
        qdir.cpp \
        qfile.cpp \
        qfileengine.cpp \
        qfileinfo.cpp \
        qglobal.cpp \
        qlibraryinfo.cpp \
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
        qurl.cpp \
        qunicodetables.cpp \
        quuid.cpp \
	qsettings.cpp \
	qvariant.cpp \
	qrect.cpp \
        qline.cpp \
	qpoint.cpp \
	qsize.cpp \
        qvector.cpp \
        qvsnprintf.cpp

   HEADERS+= \
        qbitarray.h \
        qbuffer.h \
	qbufferedfsfileengine_p.h \
        qbytearray.h \
        qbytearraymatcher.h \
        qchar.h \
        qdatastream.h \
        qdatetime.h \
        qdatetime_p.h \
        qdir.h \
        qfile.h \
        qfileengine.h \
        qfileinfo.h \
        qglobal.h \
	qlibraryinfo.h \
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
        qurl.h \
        quuid.h \
        qvector.h

    unix {
        SOURCES += qfileengine_unix.cpp
        mac {
          SOURCES += qcore_mac.cpp qsettings_mac.cpp
          QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.2 #enables weak linking for 10.2 (exported)
          LIBS += -framework CoreServices
        }
    } else:win32 {
	SOURCES += qfileengine_win.cpp qsettings_win.cpp
        win32-msvc*:LIBS += ole32.lib advapi32.lib
    }

    qnx {
        CFLAGS += -fhonor-std
        LFLAGS += -lcpp
    }
    DEFINES *= QT_NO_QOBJECT
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

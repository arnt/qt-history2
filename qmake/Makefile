!IF "$(QMAKESPEC)" == "win32-msvc" || "$(QMAKESPEC)" == "win32-msvc.net" || "$(QMAKESPEC)" == "win32-icc"
#
# specific stuff for NMake and ICC
#
!if "$(QMAKESPEC)" == "win32-icc"
CXX	    = icl
LINK	    = xilink
!else
CXX         = cl
LINK	    = link
!endif

CFLAGS	    = -c -Fo$@ \
              -W3 -nologo -O1 \
              -I. -Igenerators -Igenerators\unix -Igenerators\win32 -Igenerators\mac \
              -I..\src\core\arch\generic -I..\include -I..\src\tools -I..\include\QtCore \
              -I..\mkspecs\win32-msvc \
              -DQT_NO_TEXTCODEC -DQT_NO_UNICODETABLES -DQT_LITE_COMPONENT -DQT_NODLL -DQT_NO_STL \
              -DQT_NO_COMPRESS -DUNICODE -DHAVE_QCONFIG_CPP -DQT_BUILD_QMAKE -DQT_NO_THREAD \
              -DQT_NO_QOBJECT
CXXFLAGS    = $(CFLAGS)
LFLAGS	    =
LIBS	    = ole32.lib advapi32.lib
LINKQMAKE   = $(LINK) $(LFLAGS) -DEBUG -OUT:qmake.exe $(OBJS) $(QTOBJS) $(LIBS)
ADDCLEAN    = vc60.pdb vc70.pdb qmake.pdb qmake.ilk
!ELSE

#
# specific stuff for Borland make
#
!if !$d(BCB)
BCB = $(MAKEDIR)\..
!endif
CXX	    = bcc32
CFLAGS	    = -c -o$@ \
	-tWR -w -w-hid -w-use -O1 \
	-I. -Igenerators -Igenerators\unix -Igenerators\win32 -Igenerators\mac \
	-I..\src\core\arch\generic -I..\include -I..\src\tools -I..\include\QtCore \
	-I..\mkspecs\win32-borland \
	-DQT_NO_TEXTCODEC -DQT_NO_UNICODETABLES -DQT_LITE_COMPONENT -DQT_NODLL -DQT_NO_STL \
	-DQT_NO_COMPRESS -DUNICODE -DHAVE_QCONFIG_CPP -DQT_BUILD_QMAKE -DQT_NO_THREAD \
   	-DQT_NO_QOBJECT -DQT_NO_GEOM_VARIANT
CXXFLAGS    = $(CFLAGS)
LFLAGS	    = -L$(BCB)\lib -c -x -Gn -ap -Tpe c0x32.obj
LIBS	    = import32.lib cw32i.lib
LINKQMAKE   = ilink32 $(LFLAGS) $(OBJS) $(QTOBJS), qmake.exe,,$(LIBS)
ADDCLEAN    = qmake.tds
!ENDIF

#qmake code
OBJS        = project.obj main.obj makefile.obj unixmake.obj unixmake2.obj mingw_make.obj \
              option.obj winmakefile.obj projectgenerator.obj property.obj meta.obj qtmd5.obj \
              makefiledeps.obj metamakefile.obj xmloutput.obj

!IFNDEF QMAKE_OPENSOURCE_EDITION
OBJS        = $(OBJS) borland_bmake.obj msvc_nmake.obj msvc_dsp.obj msvc_vcproj.obj \
              msvc_objectmodel.obj metrowerks_xml.obj pbuilder_pbx.obj
!ELSE
CFLAGS      = $(CFLAGS) -DQMAKE_OPENSOURCE_EDITION
!ENDIF

#qt code
QTOBJS= \
	qbitarray.obj \
	qbuffer.obj \
	qbufferedfsfileengine.obj \
	qbytearray.obj \
        qvsnprintf.obj \
	qbytearraymatcher.obj \
	qchar.obj \
	qdatastream.obj \
	qdatetime.obj \
	qdir.obj \
	qfile.obj \
	qtemporaryfile.obj \
	qfileengine.obj \
	qfsfileengine_win.obj \
	qfsfileengine.obj \
	qfileinfo.obj \
	qglobal.obj \
	qhash.obj \
	qiodevice.obj \
	qinternal.obj \
	qlibraryinfo.obj \
	qlist.obj \
	qlocale.obj \
	qmap.obj \
	qregexp.obj \
	qstring.obj \
	qstringlist.obj \
	qstringmatcher.obj \
	qtextstream.obj \
	qunicodetables.obj \
	quuid.obj \
	qvector.obj \
	qsettings.obj \
	qvariant.obj \
        qurl.obj \
	qsettings_win.obj \
        qmetatype.obj

first all: qmake.exe

qmake.exe: $(OBJS) $(QTOBJS)
	$(LINKQMAKE)
	-copy qmake.exe ..\bin\qmake.exe

clean::
	-del qbitarray.obj
	-del qbuffer.obj
	-del qbufferedfsfileengine.obj
	-del qbytearray.obj
        -del qvsnprintf.obj
	-del qbytearraymatcher.obj
	-del qchar.obj
	-del qdatastream.obj
	-del qdatetime.obj
	-del qdir.obj
	-del qfile.obj
	-del qtemporaryfile.obj
	-del qfileengine.obj
	-del qfsfileengine_win.obj
	-del qfsfileengine.obj
	-del qfileinfo.obj
	-del qglobal.obj
	-del qhash.obj
	-del qiodevice.obj
	-del qinternal.obj
	-del qlibraryinfo.obj
	-del qlist.obj
	-del qlocale.obj
	-del qmap.obj
	-del qregexp.obj
	-del qstring.obj
	-del qstringlist.obj
	-del qstringmatcher.obj
	-del qtextstream.obj
	-del qunicodetables.obj
	-del quuid.obj
	-del qvector.obj
	-del qsettings.obj
	-del qvariant.obj
        -del qurl.obj
	-del qsettings_win.obj
        -del qmetatype.obj
	-del project.obj
	-del main.obj
	-del makefile.obj
	-del unixmake.obj
	-del unixmake2.obj
	-del mingw_make.obj
	-del option.obj
	-del winmakefile.obj
	-del projectgenerator.obj
	-del property.obj
	-del meta.obj
	-del qtmd5.obj
	-del makefiledeps.obj
	-del metamakefile.obj
	-del xmloutput.obj
	-del borland_bmake.obj
	-del msvc_nmake.obj
	-del msvc_dsp.obj
	-del msvc_vcproj.obj
	-del msvc_objectmodel.obj
	-del metrowerks_xml.obj
	-del pbuilder_pbx.obj
	-del vc60.pdb
	-del vc70.pdb
	-del qmake.pdb
	-del qmake.ilk
	-del qmake.tds

distclean:: clean
	-del qmake

.c.obj:
	$(CXX) $(CFLAGS) $<

.cpp.obj:
	$(CXX) $(CXXFLAGS) $<

.cc.obj:
	$(CXX) $(CXXFLAGS) $<

.cxx.obj:
	$(CXX) $(CXXFLAGS) $<

qsettings_win.obj: ..\src\core\io\qsettings_win.cpp
	$(CXX) $(CXXFLAGS) ..\src\core\io\qsettings_win.cpp

qsettings.obj: ..\src\core\io\qsettings.cpp
	$(CXX) $(CXXFLAGS) ..\src\core\io\qsettings.cpp

qvariant.obj: ..\src\core\kernel\qvariant.cpp
	$(CXX) $(CXXFLAGS) ..\src\core\kernel\qvariant.cpp

qurl.obj: ..\src\core\io\qurl.cpp
	$(CXX) $(CXXFLAGS) ..\src\core\io\qurl.cpp

qtextstream.obj: ..\src\core\io\qtextstream.cpp
	$(CXX) $(CXXFLAGS) ..\src\core\io\qtextstream.cpp

qiodevice.obj: ..\src\core\io\qiodevice.cpp
	$(CXX) $(CXXFLAGS) ..\src\core\io\qiodevice.cpp

qinternal.obj: ..\src\core\kernel\qinternal.cpp
	$(CXX) $(CXXFLAGS) ..\src\core\kernel\qinternal.cpp

qlibraryinfo.obj: ..\src\core\global\qlibraryinfo.cpp
	$(CXX) $(CXXFLAGS) ..\src\core\global\qlibraryinfo.cpp

qglobal.obj: ..\src\core\global\qglobal.cpp
	$(CXX) $(CXXFLAGS) ..\src\core\global\qglobal.cpp

qhash.obj: ..\src\core\tools\qhash.cpp
	$(CXX) $(CXXFLAGS) ..\src\core\tools\qhash.cpp

qbytearray.obj: ..\src\core\tools\qbytearray.cpp
	$(CXX) $(CXXFLAGS) ..\src\core\tools\qbytearray.cpp

qvsnprintf.obj: ..\src\core\tools\qvsnprintf.cpp
	$(CXX) $(CXXFLAGS) ..\src\core\tools\qvsnprintf.cpp

qbytearraymatcher.obj: ..\src\core\tools\qbytearraymatcher.cpp
	$(CXX) $(CXXFLAGS) ..\src\core\tools\qbytearraymatcher.cpp

qchar.obj: ..\src\core\tools\qchar.cpp
	$(CXX) $(CXXFLAGS) ..\src\core\tools\qchar.cpp

qstring.obj: ..\src\core\tools\qstring.cpp
	$(CXX) $(CXXFLAGS) ..\src\core\tools\qstring.cpp

qstringmatcher.obj: ..\src\core\tools\qstringmatcher.cpp
	$(CXX) $(CXXFLAGS) ..\src\core\tools\qstringmatcher.cpp

qlocale.obj: ..\src\core\tools\qlocale.cpp
	$(CXX) $(CXXFLAGS) ..\src\core\tools\qlocale.cpp

quuid.obj: ..\src\core\plugin\quuid.cpp
	$(CXX) $(CXXFLAGS) ..\src\core\plugin\quuid.cpp

qdatastream.obj: ..\src\core\io\qdatastream.cpp
	$(CXX) $(CXXFLAGS) ..\src\core\io\qdatastream.cpp

qbuffer.obj: ..\src\core\io\qbuffer.cpp
	$(CXX) $(CXXFLAGS) ..\src\core\io\qbuffer.cpp

qlist.obj: ..\src\core\tools\qlist.cpp
	$(CXX) $(CXXFLAGS) ..\src\core\tools\qlist.cpp

qlinkedlist.obj: ..\src\core\tools\qlinkedlist.cpp
	$(CXX) $(CXXFLAGS) ..\src\core\tools\qlinkedlist.cpp

qfile.obj: ..\src\core\io\qfile.cpp
	$(CXX) $(CXXFLAGS) ..\src\core\io\qfile.cpp

qtemporaryfile.obj: ..\src\core\io\qtemporaryfile.cpp
	$(CXX) $(CXXFLAGS) ..\src\core\io\qtemporaryfile.cpp

qfsfileengine_win.obj: ..\src\core\io\qfsfileengine_win.cpp
	$(CXX) $(CXXFLAGS) ..\src\core\io\qfsfileengine_win.cpp

qfsfileengine.obj: ..\src\core\io\qfsfileengine.cpp
	$(CXX) $(CXXFLAGS) ..\src\core\io\qfsfileengine.cpp

qfileengine.obj: ..\src\core\io\qfileengine.cpp
	$(CXX) $(CXXFLAGS) ..\src\core\io\qfileengine.cpp

qbufferedfsfileengine.obj: ..\src\core\io\qbufferedfsfileengine.cpp
	$(CXX) $(CXXFLAGS) ..\src\core\io\qbufferedfsfileengine.cpp

qtextcodec.obj: ..\src\codecs\qtextcodec.cpp
	$(CXX) $(CXXFLAGS) ..\src\codecs\qtextcodec.cpp

qregexp.obj: ..\src\core\tools\qregexp.cpp
	$(CXX) $(CXXFLAGS) ..\src\core\tools\qregexp.cpp

qvector.obj: ..\src\core\tools\qvector.cpp
	$(CXX) $(CXXFLAGS) ..\src\core\tools\qvector.cpp

qbitarray.obj: ..\src\core\tools\qbitarray.cpp
	$(CXX) $(CXXFLAGS) ..\src\core\tools\qbitarray.cpp

qdir.obj: ..\src\core\io\qdir.cpp
	$(CXX) $(CXXFLAGS) ..\src\core\io\qdir.cpp

qmetatype.obj: ..\src\core\kernel\qmetatype.cpp
	$(CXX) $(CXXFLAGS) ..\src\core\kernel\qmetatype.cpp

qfileinfo.obj: ..\src\core\io\qfileinfo.cpp
	$(CXX) $(CXXFLAGS) ..\src\core\io\qfileinfo.cpp

qdatetime.obj: ..\src\core\tools\qdatetime.cpp
	$(CXX) $(CXXFLAGS) ..\src\core\tools\qdatetime.cpp

qstringlist.obj: ..\src\core\tools\qstringlist.cpp
	$(CXX) $(CXXFLAGS) ..\src\core\tools\qstringlist.cpp

qmap.obj: ..\src\core\tools\qmap.cpp
	$(CXX) $(CXXFLAGS) ..\src\core\tools\qmap.cpp

qunicodetables.obj: ..\src\core\tools\qunicodetables.cpp
	$(CXX) $(CXXFLAGS) ..\src\core\tools\qunicodetables.cpp

makefile.obj: generators\makefile.cpp
	$(CXX) $(CXXFLAGS) generators\makefile.cpp

unixmake.obj: generators\unix\unixmake.cpp
	$(CXX) $(CXXFLAGS) generators\unix\unixmake.cpp

unixmake2.obj: generators\unix\unixmake2.cpp
	$(CXX) $(CXXFLAGS) generators\unix\unixmake2.cpp

winmakefile.obj: generators/win32/winmakefile.cpp
	$(CXX) $(CXXFLAGS) generators/win32/winmakefile.cpp

borland_bmake.obj: generators/win32/borland_bmake.cpp
	$(CXX) $(CXXFLAGS) generators/win32/borland_bmake.cpp

mingw_make.obj: generators/win32/mingw_make.cpp
	$(CXX) $(CXXFLAGS) generators/win32/mingw_make.cpp

msvc_nmake.obj: generators/win32/msvc_nmake.cpp
	$(CXX) $(CXXFLAGS) generators/win32/msvc_nmake.cpp

msvc_dsp.obj: generators/win32/msvc_dsp.cpp
	$(CXX) $(CXXFLAGS) generators/win32/msvc_dsp.cpp

msvc_vcproj.obj: generators/win32/msvc_vcproj.cpp
	$(CXX) $(CXXFLAGS) generators/win32/msvc_vcproj.cpp

msvc_objectmodel.obj: generators/win32/msvc_objectmodel.cpp
	$(CXX) $(CXXFLAGS) generators/win32/msvc_objectmodel.cpp

qtmd5.obj: qtmd5.cpp
	$(CXX) $(CXXFLAGS) qtmd5.cpp

project.obj: project.cpp project.h option.h
	$(CXX) $(CXXFLAGS) project.cpp

meta.obj: meta.cpp project.h option.h
	$(CXX) $(CXXFLAGS) meta.cpp

main.obj: main.cpp project.h
	$(CXX) $(CXXFLAGS) main.cpp

option.obj: option.cpp option.h
	$(CXX) $(CXXFLAGS) option.cpp

property.obj: property.cpp project.h option.h
	$(CXX) $(CXXFLAGS) property.cpp

projectgenerator.obj: generators/projectgenerator.cpp
	$(CXX) $(CXXFLAGS) generators/projectgenerator.cpp

metrowerks_xml.obj: generators/mac/metrowerks_xml.cpp
	$(CXX) $(CXXFLAGS) generators/mac/metrowerks_xml.cpp

pbuilder_pbx.obj: generators/mac/pbuilder_pbx.cpp
	$(CXX) $(CXXFLAGS) generators/mac/pbuilder_pbx.cpp

makefiledeps.obj: generators/makefiledeps.cpp
	$(CXX) $(CXXFLAGS) generators/makefiledeps.cpp

metamakefile.obj: generators/metamakefile.cpp
	$(CXX) $(CXXFLAGS) generators/metamakefile.cpp

xmloutput.obj: generators/xmloutput.cpp
	$(CXX) $(CXXFLAGS) generators/xmloutput.cpp

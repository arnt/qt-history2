!IF "$(MAKE)" == "NMAKE" || "$(MAKE)" == "Nmake" || "$(MAKE)" == "nmake"
#
# specific stuff for nmake
#
CXX	    =   cl
CFLAGS	    =	/c /Fo$@ \
		/W3 /nologo /O1 \
		/I. /Igenerators /Igenerators\unix /Igenerators\win32 /Igenerators\mac /I..\include /I..\src\tools \
		/I..\mkspecs\win32-msvc \
		/DQT_NO_TEXTCODEC  /DQT_LITE_COMPONENT /DQT_NODLL /DQT_NO_STL /DQT_NO_COMPRESS /DUNICODE
CXXFLAGS    =   $(CFLAGS)
LFLAGS	    =	/DEBUG
LIBS	    =	ole32.lib
LINKQMAKE   =	link $(LFLAGS) /OUT:qmake.exe $(OBJS) $(QTOBJS) $(LIBS)
ADDCLEAN    =	vc60.pdb qmake.pdb qmake.ilk
!ELSE
#
# specific stuff for Borland make
#
!if !$d(BCB)
BCB = $(MAKEDIR)\..
!endif
CXX	    =	bcc32
CFLAGS	    =	-c -o$@ \
		-tWR -w -w-hid -O1 \
		-I. -Igenerators -Igenerators\unix -Igenerators\win32 -Igenerators\mac -I..\include -I..\src\tools \
		-I..\mkspecs\win32-borland \
		-DQT_NO_TEXTCODEC -DQT_LITE_COMPONENT -DQT_NODLL -DQT_NO_STL -DQT_NO_COMPRESS -DUNICODE
CXXFLAGS    =   $(CFLAGS)
LFLAGS	    =	-L$(BCB)\lib -c -x -Gn -ap -Tpe c0x32.obj
LIBS	    =	import32.lib cw32i.lib
LINKQMAKE   =	ilink32 $(LFLAGS) $(OBJS) $(QTOBJS), qmake.exe,,$(LIBS)
ADDCLEAN    =	qmake.tds
!ENDIF

#qmake code
OBJS=project.obj main.obj makefile.obj unixmake.obj unixmake2.obj borland_bmake.obj msvc_nmake.obj msvc_dsp.obj msvc_vcproj.obj option.obj \
	winmakefile.obj projectgenerator.obj metrowerks_xml.obj pbuilder_pbx.obj

#qt code
QTOBJS=qstring.obj qtextstream.obj qiodevice.obj qglobal.obj qgdict.obj qcstring.obj qdatastream.obj qgarray.obj qbuffer.obj \
	qglist.obj qptrcollection.obj qfile.obj qfile_win.obj qregexp.obj qgvector.obj qgcache.obj qbitarray.obj \
	qdir.obj qfileinfo_win.obj qdir_win.obj qfileinfo.obj qdatetime.obj qstringlist.obj qmap.obj qlibrary.obj qlibrary_win.obj \
        qconfig.obj



qmake.exe: $(OBJS) $(QTOBJS)
	$(LINKQMAKE)
	-copy qmake.exe ..\bin\qmake.exe

clean::
	-del $(OBJS) $(QTOBJS) $(ADDCLEAN)

distclean:: clean
	-del qmake

.c.obj:
	$(CXX) $(CFLAGS) $<

.cpp.obj:
	$(CXX) $(CXXFLAGS) $<

qconfig.obj: ..\src\tools\qconfig.cpp
	$(CXX) $(CXXFLAGS) ..\src\tools\qconfig.cpp

qlibrary.obj: ..\src\tools\qlibrary.cpp
	$(CXX) $(CXXFLAGS) ..\src\tools\qlibrary.cpp

qlibrary_win.obj: ..\src\tools\qlibrary_win.cpp
	$(CXX) $(CXXFLAGS) ..\src\tools\qlibrary_win.cpp

qtextstream.obj: ..\src\tools\qtextstream.cpp
	$(CXX) $(CXXFLAGS) ..\src\tools\qtextstream.cpp

qiodevice.obj: ..\src\tools\qiodevice.cpp
	$(CXX) $(CXXFLAGS) ..\src\tools\qiodevice.cpp

qglobal.obj: ..\src\tools\qglobal.cpp
	$(CXX) $(CXXFLAGS) ..\src\tools\qglobal.cpp

qgdict.obj: ..\src\tools\qgdict.cpp
	$(CXX) $(CXXFLAGS) ..\src\tools\qgdict.cpp

qcstring.obj: ..\src\tools\qcstring.cpp
	$(CXX) $(CXXFLAGS) ..\src\tools\qcstring.cpp

qstring.obj: ..\src\tools\qstring.cpp
	$(CXX) $(CXXFLAGS) ..\src\tools\qstring.cpp

qdatastream.obj: ..\src\tools\qdatastream.cpp
	$(CXX) $(CXXFLAGS) ..\src\tools\qdatastream.cpp

qgarray.obj: ..\src\tools\qgarray.cpp
	$(CXX) $(CXXFLAGS) ..\src\tools\qgarray.cpp

qbuffer.obj: ..\src\tools\qbuffer.cpp
	$(CXX) $(CXXFLAGS) ..\src\tools\qbuffer.cpp

qglist.obj: ..\src\tools\qglist.cpp
	$(CXX) $(CXXFLAGS) ..\src\tools\qglist.cpp

qptrcollection.obj: ..\src\tools\qptrcollection.cpp
	$(CXX) $(CXXFLAGS) ..\src\tools\qptrcollection.cpp

qfile.obj: ..\src\tools\qfile.cpp
	$(CXX) $(CXXFLAGS) ..\src\tools\qfile.cpp

qfile_win.obj: ..\src\tools\qfile_win.cpp
	$(CXX) $(CXXFLAGS) ..\src\tools\qfile_win.cpp

qtextcodec.obj: ..\src\codecs\qtextcodec.cpp
	$(CXX) $(CXXFLAGS) ..\src\codecs\qtextcodec.cpp

qregexp.obj: ..\src\tools\qregexp.cpp
	$(CXX) $(CXXFLAGS) ..\src\tools\qregexp.cpp

qgvector.obj: ..\src\tools\qgvector.cpp
	$(CXX) $(CXXFLAGS) ..\src\tools\qgvector.cpp

qgcache.obj: ..\src\tools\qgcache.cpp
	$(CXX) $(CXXFLAGS) ..\src\tools\qgcache.cpp

qbitarray.obj: ..\src\tools\qbitarray.cpp
	$(CXX) $(CXXFLAGS) ..\src\tools\qbitarray.cpp

qdir.obj: ..\src\tools\qdir.cpp
	$(CXX) $(CXXFLAGS) ..\src\tools\qdir.cpp

qfileinfo_win.obj: ..\src\tools\qfileinfo_win.cpp
	$(CXX) $(CXXFLAGS) ..\src\tools\qfileinfo_win.cpp

qdir_win.obj: ..\src\tools\qdir_win.cpp
	$(CXX) $(CXXFLAGS) ..\src\tools\qdir_win.cpp

qfileinfo.obj: ..\src\tools\qfileinfo.cpp
	$(CXX) $(CXXFLAGS) ..\src\tools\qfileinfo.cpp

qdatetime.obj: ..\src\tools\qdatetime.cpp
	$(CXX) $(CXXFLAGS) ..\src\tools\qdatetime.cpp

qstringlist.obj: ..\src\tools\qstringlist.cpp
	$(CXX) $(CXXFLAGS) ..\src\tools\qstringlist.cpp

qmap.obj: ..\src\tools\qmap.cpp
	$(CXX) $(CXXFLAGS) ..\src\tools\qmap.cpp

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

msvc_nmake.obj: generators/win32/msvc_nmake.cpp
	$(CXX) $(CXXFLAGS) generators/win32/msvc_nmake.cpp

msvc_dsp.obj: generators/win32/msvc_dsp.cpp
	$(CXX) $(CXXFLAGS) generators/win32/msvc_dsp.cpp

msvc_vcproj.obj: generators/win32/msvc_vcproj.cpp
	$(CXX) $(CXXFLAGS) generators/win32/msvc_vcproj.cpp

project.obj: project.cpp project.h option.h
	$(CXX) $(CXXFLAGS) project.cpp

main.obj: main.cpp project.h
	$(CXX) $(CXXFLAGS) main.cpp

option.obj: option.cpp option.h
	$(CXX) $(CXXFLAGS) option.cpp

projectgenerator.obj: generators/projectgenerator.cpp
	$(CXX) $(CXXFLAGS) generators/projectgenerator.cpp

metrowerks_xml.obj: generators/mac/metrowerks_xml.cpp
	$(CXX) $(CXXFLAGS) generators/mac/metrowerks_xml.cpp

pbuilder_pbx.obj: generators/mac/pbuilder_pbx.cpp
	$(CXX) $(CXXFLAGS) generators/mac/pbuilder_pbx.cpp

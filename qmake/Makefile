#qmake code
OBJS=project.obj main.obj makefile.obj unixmake.obj borland_bmake.obj msvc_nmake.obj msvc_dsp.obj option.obj \
	winmakefile.obj

#qt code
QTOBJS=qstring.obj qtextstream.obj qiodevice.obj qglobal.obj qgdict.obj qcstring.obj qdatastream.obj qgarray.obj qbuffer.obj \
	qglist.obj qcollection.obj qfile.obj qfile_win.obj qregexp.obj qgvector.obj qgcache.obj qbitarray.obj \
	qdir.obj qfileinfo_win.obj qdir_win.obj qfileinfo.obj qdatetime.obj qstringlist.obj qmap.obj qlibrary.obj qtextcodec.obj

CFLAGS=/W3 /Zi /nologo /I. /Igenerators /Igenerators\unix /Igenerators\win32  /I$(QTDIR)\include /I$(QTDIR)\src\tools \
	/DQT_NO_CODECS /DQT_LITE_COMPONENT /DQT_NODLL /DUSE_GROSS_BIG_BUFFER_THING
LFLAGS=/DEBUG
LIBS=ole32.lib

qmake: $(OBJS) $(QTOBJS)
	link $(LFLAGS) /OUT:qmake.exe $(OBJS) $(QTOBJS) $(LIBS)
	-copy qmake.exe $(QTDIR)\bin\qmake.exe

clean::
	-del $(OBJS) $(QTOBJS)

distclean:: clean
	-del qmake 

.c.obj: 
	$(CXX) /c /Fo$@ $(CFLAGS) $<

.cpp.obj: 
	$(CXX) /c /Fo$@ $(CFLAGS) $<

qtextstream.obj: $(QTDIR)\src\tools\qtextstream.cpp
	$(CXX) /c /Fo$@ $(CFLAGS) $(QTDIR)\src\tools\qtextstream.cpp

qiodevice.obj: $(QTDIR)\src\tools\qiodevice.cpp
	$(CXX) /c /Fo$@ $(CFLAGS) $(QTDIR)\src\tools\qiodevice.cpp

qglobal.obj: $(QTDIR)\src\tools\qglobal.cpp
	$(CXX) /c /Fo$@ $(CFLAGS) $(QTDIR)\src\tools\qglobal.cpp

qgdict.obj: $(QTDIR)\src\tools\qgdict.cpp
	$(CXX) /c /Fo$@ $(CFLAGS) $(QTDIR)\src\tools\qgdict.cpp

qcstring.obj: $(QTDIR)\src\tools\qcstring.cpp
	$(CXX) /c /Fo$@ $(CFLAGS) $(QTDIR)\src\tools\qcstring.cpp

qstring.obj: $(QTDIR)\src\tools\qstring.cpp
	$(CXX) /c /Fo$@ $(CFLAGS) $(QTDIR)\src\tools\qstring.cpp

qdatastream.obj: $(QTDIR)\src\tools\qdatastream.cpp
	$(CXX) /c /Fo$@ $(CFLAGS) $(QTDIR)\src\tools\qdatastream.cpp

qgarray.obj: $(QTDIR)\src\tools\qgarray.cpp
	$(CXX) /c /Fo$@ $(CFLAGS) $(QTDIR)\src\tools\qgarray.cpp

qbuffer.obj: $(QTDIR)\src\tools\qbuffer.cpp
	$(CXX) /c /Fo$@ $(CFLAGS) $(QTDIR)\src\tools\qbuffer.cpp

qglist.obj: $(QTDIR)\src\tools\qglist.cpp
	$(CXX) /c /Fo$@ $(CFLAGS) $(QTDIR)\src\tools\qglist.cpp

qcollection.obj: $(QTDIR)\src\tools\qcollection.cpp
	$(CXX) /c /Fo$@ $(CFLAGS) $(QTDIR)\src\tools\qcollection.cpp

qfile.obj: $(QTDIR)\src\tools\qfile.cpp
	$(CXX) /c /Fo$@ $(CFLAGS) $(QTDIR)\src\tools\qfile.cpp

qfile_win.obj: $(QTDIR)\src\tools\qfile_win.cpp
	$(CXX) /c /Fo$@ $(CFLAGS) $(QTDIR)\src\tools\qfile_win.cpp

qtextcodec.obj: $(QTDIR)\src\codecs\qtextcodec.cpp
	$(CXX) /c /Fo$@ $(CFLAGS) $(QTDIR)\src\codecs\qtextcodec.cpp

qregexp.obj: $(QTDIR)\src\tools\qregexp.cpp
	$(CXX) /c /Fo$@ $(CFLAGS) $(QTDIR)\src\tools\qregexp.cpp

qgvector.obj: $(QTDIR)\src\tools\qgvector.cpp
	$(CXX) /c /Fo$@ $(CFLAGS) $(QTDIR)\src\tools\qgvector.cpp

qgcache.obj: $(QTDIR)\src\tools\qgcache.cpp
	$(CXX) /c /Fo$@ $(CFLAGS) $(QTDIR)\src\tools\qgcache.cpp

qbitarray.obj: $(QTDIR)\src\tools\qbitarray.cpp
	$(CXX) /c /Fo$@ $(CFLAGS) $(QTDIR)\src\tools\qbitarray.cpp

qdir.obj: $(QTDIR)\src\tools\qdir.cpp
	$(CXX) /c /Fo$@ $(CFLAGS) $(QTDIR)\src\tools\qdir.cpp

qfileinfo_win.obj: $(QTDIR)\src\tools\qfileinfo_win.cpp
	$(CXX) /c /Fo$@ $(CFLAGS) $(QTDIR)\src\tools\qfileinfo_win.cpp

qdir_win.obj: $(QTDIR)\src\tools\qdir_win.cpp
	$(CXX) /c /Fo$@ $(CFLAGS) $(QTDIR)\src\tools\qdir_win.cpp

qfileinfo.obj: $(QTDIR)\src\tools\qfileinfo.cpp
	$(CXX) /c /Fo$@ $(CFLAGS) $(QTDIR)\src\tools\qfileinfo.cpp

qdatetime.obj: $(QTDIR)\src\tools\qdatetime.cpp
	$(CXX) /c /Fo$@ $(CFLAGS) $(QTDIR)\src\tools\qdatetime.cpp

qlibrary.obj: $(QTDIR)\src\kernel\qlibrary.cpp
	$(CXX) /c /Fo$@ $(CFLAGS) $(QTDIR)\src\kernel\qlibrary.cpp

qstringlist.obj: $(QTDIR)\src\tools\qstringlist.cpp
	$(CXX) /c /Fo$@ $(CFLAGS) $(QTDIR)\src\tools\qstringlist.cpp

qmap.obj: $(QTDIR)\src\tools\qmap.cpp
	$(CXX) /c /Fo$@ $(CFLAGS) $(QTDIR)\src\tools\qmap.cpp

makefile.obj: generators\makefile.cpp
	$(CXX) /c /Fo$@ $(CFLAGS) generators\makefile.cpp

unixmake.obj: generators\unix\unixmake.cpp
	$(CXX) /c /Fo$@ $(CFLAGS) generators\unix\unixmake.cpp

winmakefile.obj: generators/win32/winmakefile.cpp
	$(CXX) /c /Fo$@ $(CFLAGS) generators/win32/winmakefile.cpp

borland_bmake.obj: generators/win32/borland_bmake.cpp
	$(CXX) /c /Fo$@ $(CFLAGS) generators/win32/borland_bmake.cpp

msvc_nmake.obj: generators/win32/msvc_nmake.cpp
	$(CXX) /c /Fo$@ $(CFLAGS) generators/win32/msvc_nmake.cpp

msvc_dsp.obj: generators/win32/msvc_dsp.cpp
	$(CXX) /c /Fo$@ $(CFLAGS) generators/win32/msvc_dsp.cpp

project.obj: project.cpp project.h option.h
	$(CXX) /c /Fo$@ $(CFLAGS) project.cpp

main.obj: main.cpp project.h
	$(CXX) /c /Fo$@ $(CFLAGS) main.cpp

option.obj: option.cpp option.h
	$(CXX) /c /Fo$@ $(CFLAGS) option.cpp


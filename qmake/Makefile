#qmake code
OBJS=project.obj main.obj makefile.obj unixmake.obj borland_bmake.obj msvc_nmake.obj msvc_dsp.obj option.obj \
	winmakefile.obj projectgenerator.obj metrowerks_xml.obj

#qt code
QTOBJS=qstring.obj qtextstream.obj qiodevice.obj qglobal.obj qgdict.obj qcstring.obj qdatastream.obj qgarray.obj qbuffer.obj \
	qglist.obj qptrcollection.obj qfile.obj qfile_win.obj qregexp.obj qgvector.obj qgcache.obj qbitarray.obj \
	qdir.obj qfileinfo_win.obj qdir_win.obj qfileinfo.obj qdatetime.obj qstringlist.obj qmap.obj qlibrary.obj qtextcodec.obj


!IF "$(MAKE)" == "NMAKE" || "$(MAKE)" == "Nmake" || "$(MAKE)" == "nmake"
CFLAGS	=	/c /Fo$@ \
		/W3 /Zi /nologo /GX /I. /Igenerators /Igenerators\unix /Igenerators\win32  /I$(QTDIR)\include /I$(QTDIR)\src\tools \
		/DQT_NO_CODECS /DQT_LITE_COMPONENT /DQT_NODLL
LFLAGS	=	/DEBUG
LIBS	=	ole32.lib
LINKQMAKE =	link $(LFLAGS) /OUT:qmake.exe $(OBJS) $(QTOBJS) $(LIBS)
!ELSE
CXX	=	bcc32
CFLAGS	=	-c -o$@ \
		-tWR -w -O2 -DQT_NO_TEXTCODEC -DQT_LITE_COMPONENT -DQT_NODLL \
		-I. -Igenerators -Igenerators\unix -Igenerators\win32  -I$(QTDIR)\include -I$(QTDIR)\src\tools
LINK	=	ilink32
LFLAGS	=	-L$(BCB)\lib -c -x -Gn -ap -Tpe c0x32.obj
LIBS	=	import32.lib cw32i.lib
LINKQMAKE =	$(LINK) $(LFLAGS) $(OBJS) $(QTOBJS), qmake.exe,,$(LIBS)
!ENDIF


qmake.exe: $(OBJS) $(QTOBJS)
	$(LINKQMAKE)
	-copy qmake.exe $(QTDIR)\bin\qmake.exe

clean::
	-del project.obj
	-del main.obj
	-del makefile.obj 
	-del unixmake.obj
	-del borland_bmake.obj
	-del msvc_nmake.obj
	-del msvc_dsp.obj
	-del option.obj
	-del winmakefile.obj
	-del projectgenerator.obj
	-del metrowerks_xml.obj
	-del qstring.obj
	-del qtextstream.obj
	-del qiodevice.obj
	-del qglobal.obj
	-del qgdict.obj
	-del qcstring.obj
	-del qdatastream.obj
	-del qgarray.obj
	-del qbuffer.obj
	-del qglist.obj
	-del qptrcollection.obj
	-del qfile.obj
	-del qfile_win.obj
	-del qregexp.obj
	-del qgvector.obj
	-del qgcache.obj
	-del qbitarray.obj
	-del qdir.obj
	-del qfileinfo_win.obj
	-del qdir_win.obj
	-del qfileinfo.obj
	-del qdatetime.obj
	-del qstringlist.obj
	-del qmap.obj
	-del qlibrary.obj
	-del qtextcodec.obj

distclean:: clean
	-del qmake 

.c.obj: 
	$(CXX) $(CFLAGS) $<

.cpp.obj: 
	$(CXX) $(CFLAGS) $<

qtextstream.obj: $(QTDIR)\src\tools\qtextstream.cpp
	$(CXX) $(CFLAGS) $(QTDIR)\src\tools\qtextstream.cpp

qiodevice.obj: $(QTDIR)\src\tools\qiodevice.cpp
	$(CXX) $(CFLAGS) $(QTDIR)\src\tools\qiodevice.cpp

qglobal.obj: $(QTDIR)\src\tools\qglobal.cpp
	$(CXX) $(CFLAGS) $(QTDIR)\src\tools\qglobal.cpp

qgdict.obj: $(QTDIR)\src\tools\qgdict.cpp
	$(CXX) $(CFLAGS) $(QTDIR)\src\tools\qgdict.cpp

qcstring.obj: $(QTDIR)\src\tools\qcstring.cpp
	$(CXX) $(CFLAGS) $(QTDIR)\src\tools\qcstring.cpp

qstring.obj: $(QTDIR)\src\tools\qstring.cpp
	$(CXX) $(CFLAGS) $(QTDIR)\src\tools\qstring.cpp

qdatastream.obj: $(QTDIR)\src\tools\qdatastream.cpp
	$(CXX) $(CFLAGS) $(QTDIR)\src\tools\qdatastream.cpp

qgarray.obj: $(QTDIR)\src\tools\qgarray.cpp
	$(CXX) $(CFLAGS) $(QTDIR)\src\tools\qgarray.cpp

qbuffer.obj: $(QTDIR)\src\tools\qbuffer.cpp
	$(CXX) $(CFLAGS) $(QTDIR)\src\tools\qbuffer.cpp

qglist.obj: $(QTDIR)\src\tools\qglist.cpp
	$(CXX) $(CFLAGS) $(QTDIR)\src\tools\qglist.cpp

qptrcollection.obj: $(QTDIR)\src\tools\qptrcollection.cpp
	$(CXX) $(CFLAGS) $(QTDIR)\src\tools\qptrcollection.cpp

qfile.obj: $(QTDIR)\src\tools\qfile.cpp
	$(CXX) $(CFLAGS) $(QTDIR)\src\tools\qfile.cpp

qfile_win.obj: $(QTDIR)\src\tools\qfile_win.cpp
	$(CXX) $(CFLAGS) $(QTDIR)\src\tools\qfile_win.cpp

qtextcodec.obj: $(QTDIR)\src\codecs\qtextcodec.cpp
	$(CXX) $(CFLAGS) $(QTDIR)\src\codecs\qtextcodec.cpp

qregexp.obj: $(QTDIR)\src\tools\qregexp.cpp
	$(CXX) $(CFLAGS) $(QTDIR)\src\tools\qregexp.cpp

qgvector.obj: $(QTDIR)\src\tools\qgvector.cpp
	$(CXX) $(CFLAGS) $(QTDIR)\src\tools\qgvector.cpp

qgcache.obj: $(QTDIR)\src\tools\qgcache.cpp
	$(CXX) $(CFLAGS) $(QTDIR)\src\tools\qgcache.cpp

qbitarray.obj: $(QTDIR)\src\tools\qbitarray.cpp
	$(CXX) $(CFLAGS) $(QTDIR)\src\tools\qbitarray.cpp

qdir.obj: $(QTDIR)\src\tools\qdir.cpp
	$(CXX) $(CFLAGS) $(QTDIR)\src\tools\qdir.cpp

qfileinfo_win.obj: $(QTDIR)\src\tools\qfileinfo_win.cpp
	$(CXX) $(CFLAGS) $(QTDIR)\src\tools\qfileinfo_win.cpp

qdir_win.obj: $(QTDIR)\src\tools\qdir_win.cpp
	$(CXX) $(CFLAGS) $(QTDIR)\src\tools\qdir_win.cpp

qfileinfo.obj: $(QTDIR)\src\tools\qfileinfo.cpp
	$(CXX) $(CFLAGS) $(QTDIR)\src\tools\qfileinfo.cpp

qdatetime.obj: $(QTDIR)\src\tools\qdatetime.cpp
	$(CXX) $(CFLAGS) $(QTDIR)\src\tools\qdatetime.cpp

qlibrary.obj: $(QTDIR)\src\tools\qlibrary.cpp
	$(CXX) $(CFLAGS) $(QTDIR)\src\tools\qlibrary.cpp

qstringlist.obj: $(QTDIR)\src\tools\qstringlist.cpp
	$(CXX) $(CFLAGS) $(QTDIR)\src\tools\qstringlist.cpp

qmap.obj: $(QTDIR)\src\tools\qmap.cpp
	$(CXX) $(CFLAGS) $(QTDIR)\src\tools\qmap.cpp

makefile.obj: generators\makefile.cpp
	$(CXX) $(CFLAGS) generators\makefile.cpp

unixmake.obj: generators\unix\unixmake.cpp
	$(CXX) $(CFLAGS) generators\unix\unixmake.cpp

winmakefile.obj: generators/win32/winmakefile.cpp
	$(CXX) $(CFLAGS) generators/win32/winmakefile.cpp

borland_bmake.obj: generators/win32/borland_bmake.cpp
	$(CXX) $(CFLAGS) generators/win32/borland_bmake.cpp

msvc_nmake.obj: generators/win32/msvc_nmake.cpp
	$(CXX) $(CFLAGS) generators/win32/msvc_nmake.cpp

msvc_dsp.obj: generators/win32/msvc_dsp.cpp
	$(CXX) $(CFLAGS) generators/win32/msvc_dsp.cpp

project.obj: project.cpp project.h option.h
	$(CXX) $(CFLAGS) project.cpp

main.obj: main.cpp project.h
	$(CXX) $(CFLAGS) main.cpp

option.obj: option.cpp option.h
	$(CXX) $(CFLAGS) option.cpp

projectgenerator.obj: generators/projectgenerator.cpp
	$(CXX) $(CFLAGS) generators/projectgenerator.cpp

metrowerks_xml.obj: generators/mac/metrowerks_xml.cpp
	$(CXX) $(CFLAGS) generators/mac/metrowerks_xml.cpp

# Qt tools module

tools {
	win32:TOOLS_H	= ../include
	unix:TOOLS_H	= tools
	unix:DEPENDPATH	+= :$$TOOLS_H
	TOOLS_P		= tools
	HEADERS +=  $$TOOLS_H/qarray.h \
		  $$TOOLS_H/qasciicache.h \
		  $$TOOLS_H/qasciidict.h \
		  $$TOOLS_H/qbig5codec.h \
		  $$TOOLS_H/qbitarray.h \
		  $$TOOLS_H/qbuffer.h \
		  $$TOOLS_H/qcache.h \
		  $$TOOLS_H/qcollection.h \
		  $$TOOLS_H/qcstring.h \
		  $$TOOLS_H/qdatastream.h \
		  $$TOOLS_H/qdatetime.h \
		  $$TOOLS_H/qdict.h \
		  $$TOOLS_H/qdir.h \
		  $$TOOLS_H/qeucjpcodec.h \
		  $$TOOLS_H/qeuckrcodec.h \
		  $$TOOLS_H/qfile.h \
		  $$TOOLS_P/qfiledefs_p.h \
		  $$TOOLS_H/qfileinfo.h \
		  $$TOOLS_H/qgarray.h \
		  $$TOOLS_H/qgbkcodec.h \
		  $$TOOLS_H/qgcache.h \
		  $$TOOLS_H/qgdict.h \
		  $$TOOLS_H/qgeneric.h \
		  $$TOOLS_H/qglist.h \
		  $$TOOLS_H/qglobal.h \
		  $$TOOLS_H/qgvector.h \
		  $$TOOLS_H/qintcache.h \
		  $$TOOLS_H/qintdict.h \
		  $$TOOLS_H/qiodevice.h \
		  $$TOOLS_H/qjiscodec.h \
		  $$TOOLS_H/qjpunicode.h \
		  $$TOOLS_H/qlist.h \
		  $$TOOLS_H/qmap.h \
		  $$TOOLS_H/qptrdict.h \
		  $$TOOLS_H/qqueue.h \
		  $$TOOLS_H/qregexp.h \
		  $$TOOLS_H/qrtlcodec.h \
		  $$TOOLS_H/qshared.h \
		  $$TOOLS_H/qsjiscodec.h \
		  $$TOOLS_H/qsortedlist.h \
		  $$TOOLS_H/qstack.h \
		  $$TOOLS_H/qstring.h \
		  $$TOOLS_H/qstringlist.h \
		  $$TOOLS_H/qstrlist.h \
		  $$TOOLS_H/qstrvec.h \
		  $$TOOLS_H/qtextcodec.h \
		  $$TOOLS_H/qtextstream.h \
		  $$TOOLS_H/qtsciicodec.h \
		  $$TOOLS_H/qutfcodec.h \
		  $$TOOLS_H/qvector.h \
	          $$TOOLS_H/qvaluelist.h


	win32:SOURCES += tools/qdir_win.cpp \
	 	  tools/qfile_win.cpp \
		  tools/qfileinfo_win.cpp

	unix:SOURCES += tools/qdir_unix.cpp \
		  tools/qfile_unix.cpp \
		  tools/qfileinfo_unix.cpp

	SOURCES += tools/qbig5codec.cpp \
		  tools/qbitarray.cpp \
		  tools/qbuffer.cpp \
		  tools/qcollection.cpp \
		  tools/qcstring.cpp \
		  tools/qdatastream.cpp \
		  tools/qdatetime.cpp \
		  tools/qdir.cpp \
		  tools/qeucjpcodec.cpp \
		  tools/qeuckrcodec.cpp \
		  tools/qfile.cpp \
		  tools/qfileinfo.cpp \
		  tools/qgarray.cpp \
		  tools/qgbkcodec.cpp \
		  tools/qgcache.cpp \
		  tools/qgdict.cpp \
		  tools/qglist.cpp \
		  tools/qglobal.cpp \
		  tools/qgvector.cpp \
		  tools/qiodevice.cpp \
		  tools/qjiscodec.cpp \
		  tools/qjpunicode.cpp \
		  tools/qmap.cpp \
		  tools/qregexp.cpp \
		  tools/qrtlcodec.cpp \
		  tools/qsjiscodec.cpp \
		  tools/qstring.cpp \
		  tools/qstringlist.cpp \
		  tools/qtextcodec.cpp \
		  tools/qtextstream.cpp \
		  tools/qtsciicodec.cpp \
		  tools/qutfcodec.cpp


}
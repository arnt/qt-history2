TEMPLATE	= app
CONFIG		= warn_on console release qtinc
DEFINES		+= QT_NO_CODECS QT_LITE_UNICODE QT_NO_COMPONENT QT_NODLL QT_CLEAN_NAMESPACE
INCLUDEPATH	= ../../include
DEPENDPATH	= ../../include
unix:INCLUDEPATH= $(QTDIR)/include
unix:DEPENDPATH	= $(QTDIR)/include
OBJECTS_DIR	= .
HEADERS		= binarywriter.h \
		  bookparser.h \
		  codechunk.h \
		  codeprocessor.h \
		  config.h \
		  decl.h \
		  declresolver.h \
		  doc.h \
		  emitter.h \
		  english.h \
		  htmlchunk.h \
		  htmlwriter.h \
		  location.h \
		  messages.h \
		  metaresolver.h \
		  parsehelpers.h \
		  resolver.h \
		  stringset.h \
		  tokenizer.h \
		  walkthrough.h
SOURCES		= binarywriter.cpp \
		  bookparser.cpp \
		  codechunk.cpp \
		  codeprocessor.cpp \
		  config.cpp \
		  cppparser.cpp \
		  decl.cpp \
		  declresolver.cpp \
		  doc.cpp \
		  emitter.cpp \
		  english.cpp \
		  htmlchunk.cpp \
		  htmlparser.cpp \
		  htmlwriter.cpp \
		  location.cpp \
		  main.cpp \
		  messages.cpp \
		  metaresolver.cpp \
		  parsehelpers.cpp \
		  resolver.cpp \
		  stringset.cpp \
		  tokenizer.cpp \
		  walkthrough.cpp \
		  ../../src/codecs/qtextcodec.cpp \
		  ../../src/codecs/qutfcodec.cpp \
		  ../../src/tools/qbitarray.cpp \
		  ../../src/tools/qbuffer.cpp \
		  ../../src/tools/qptrcollection.cpp \
		  ../../src/tools/qcstring.cpp \
		  ../../src/tools/qdatastream.cpp \
		  ../../src/tools/qdatetime.cpp \
		  ../../src/tools/qdir.cpp \
		  ../../src/tools/qfile.cpp \
		  ../../src/tools/qfileinfo.cpp \
		  ../../src/tools/qgarray.cpp \
		  ../../src/tools/qgcache.cpp \
		  ../../src/tools/qgdict.cpp \
		  ../../src/tools/qglist.cpp \
		  ../../src/tools/qglobal.cpp \
		  ../../src/tools/qgvector.cpp \
		  ../../src/tools/qiodevice.cpp \
		  ../../src/tools/qmap.cpp \
		  ../../src/tools/qregexp.cpp \
		  ../../src/tools/qstring.cpp \
		  ../../src/tools/qstringlist.cpp \
		  ../../src/tools/qtextstream.cpp
unix:SOURCES	+= ../../src/tools/qdir_unix.cpp \
		  ../../src/tools/qfile_unix.cpp \
		  ../../src/tools/qfileinfo_unix.cpp
win32:SOURCES	+= ../../src/tools/qdir_win.cpp \
		  ../../src/tools/qfile_win.cpp \
		  ../../src/tools/qfileinfo_win.cpp
win32:LIBS	+= ole32.lib
TARGET		= qdoc

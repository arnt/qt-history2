TEMPLATE	= app
CONFIG		= warn_on console release qtinc dylib debug
isEmpty(QT_SOURCE_TREE):QT_SOURCE_TREE=$(QTDIR)
isEmpty(QT_BUILD_TREE):QT_BUILD_TREE=$$QT_SOURCE_TREE
DEFINES		+= QT_NO_CODECS QT_NO_UNICODETABLES QT_NO_COMPONENT QT_NODLL QT_CLEAN_NAMESPACE QT_NO_STL QT_NO_COMPRESS
INCLUDEPATH	= $$QT_BUILD_TREE/include $$QT_SOURCE_TREE/include $$QT_SOURCE_TREE/arch/$$ARCH/
DEPENDPATH	= $$QT_BUILD_TREE/include $$QT_SOURCE_TREE/include
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
		  trool.h \
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
		  $$QT_SOURCE_TREE/src/codecs/qtextcodec.cpp \
		  $$QT_SOURCE_TREE/src/codecs/qutfcodec.cpp \
		  $$QT_SOURCE_TREE/src/compat/qcstring.cpp \
		  $$QT_SOURCE_TREE/src/compat/qgarray.cpp \
		  $$QT_SOURCE_TREE/src/compat/qgcache.cpp \
		  $$QT_SOURCE_TREE/src/compat/qgdict.cpp \
		  $$QT_SOURCE_TREE/src/compat/qglist.cpp \
		  $$QT_SOURCE_TREE/src/compat/qgvector.cpp \
		  $$QT_SOURCE_TREE/src/compat/qptrcollection.cpp \
		  $$QT_SOURCE_TREE/src/tools/qbitarray.cpp \
		  $$QT_SOURCE_TREE/src/tools/qbuffer.cpp \
		  $$QT_SOURCE_TREE/src/tools/qbytearray.cpp \
		  $$QT_SOURCE_TREE/src/tools/qchar.cpp \
		  $$QT_SOURCE_TREE/src/tools/qdatastream.cpp \
		  $$QT_SOURCE_TREE/src/tools/qdatetime.cpp \
		  $$QT_SOURCE_TREE/src/tools/qdir.cpp \
		  $$QT_SOURCE_TREE/src/tools/qfile.cpp \
		  $$QT_SOURCE_TREE/src/tools/qfileinfo.cpp \
		  $$QT_SOURCE_TREE/src/tools/qglobal.cpp \
		  $$QT_SOURCE_TREE/src/tools/qhash.cpp \
		  $$QT_SOURCE_TREE/src/tools/qiodevice.cpp \
		  $$QT_SOURCE_TREE/src/tools/qlibrary.cpp \
		  $$QT_SOURCE_TREE/src/tools/qlist.cpp \
		  $$QT_SOURCE_TREE/src/tools/qlinkedlist.cpp \
		  $$QT_SOURCE_TREE/src/tools/qlocale.cpp \
		  $$QT_SOURCE_TREE/src/tools/qmap.cpp \
		  $$QT_SOURCE_TREE/src/tools/qregexp.cpp \
		  $$QT_SOURCE_TREE/src/tools/qstack.cpp \
		  $$QT_SOURCE_TREE/src/tools/qstring.cpp \
		  $$QT_SOURCE_TREE/src/tools/qstringlist.cpp \
		  $$QT_SOURCE_TREE/src/tools/qtextstream.cpp \
                  $$QT_SOURCE_TREE/src/tools/qunicodetables.cpp \
                  $$QT_SOURCE_TREE/src/tools/qvector.cpp
unix:SOURCES	+= $$QT_SOURCE_TREE/src/tools/qdir_unix.cpp \
		  $$QT_SOURCE_TREE/src/tools/qfile_unix.cpp \
		  $$QT_SOURCE_TREE/src/tools/qfileinfo_unix.cpp
mac {
    SOURCES       += $$QT_SOURCE_TREE/src/tools/qlibrary_mac.cpp
    LIBS += -framework Carbon -framework QuickTime
} else:unix:SOURCES += $$QT_SOURCE_TREE/src/tools/qlibrary_unix.cpp
win32 {
   SOURCES	+= $$QT_SOURCE_TREE/src/tools/qdir_win.cpp \
	  	   $$QT_SOURCE_TREE/src/tools/qfile_win.cpp \
		   $$QT_SOURCE_TREE/src/tools/qfileinfo_win.cpp \
		   $$QT_SOURCE_TREE/src/tools/qlibrary_win.cpp
   !win32-borland:LIBS += -lole32
}

include($$QT_SOURCE_TREE/arch/$$ARCH/arch.pri)

TARGET		= qdoc

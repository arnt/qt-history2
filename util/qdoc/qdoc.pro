TEMPLATE        = app
CONFIG          = warn_on console release qtinc dylib debug
isEmpty(QT_SOURCE_TREE):QT_SOURCE_TREE=$(QTDIR)
isEmpty(QT_BUILD_TREE):QT_BUILD_TREE=$$QT_SOURCE_TREE
DEFINES         += QT_NO_CODECS QT_NO_UNICODETABLES QT_NO_COMPONENT QT_NODLL QT_CLEAN_NAMESPACE QT_NO_STL QT_NO_COMPRESS QT_COMPAT QT_NO_THREAD QT_NO_QFILE_QOBJECT

mac:DEFINES    += DO_HEADER_DOC
INCLUDEPATH     = $$QT_BUILD_TREE/include $$QT_SOURCE_TREE/include $$QT_SOURCE_TREE/src/core/arch/$$ARCH/ \
                  $$QT_BUILD_TREE/include/QtCore $$QT_SOURCE_TREE/include/QtCore \
                  $$QT_BUILD_TREE/include/Qt3Compat $$QT_SOURCE_TREE/include/Qt3Compat
DEPENDPATH      = $$INCLUDEPATH
OBJECTS_DIR     = .
HEADERS         = binarywriter.h \
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
SOURCES         = binarywriter.cpp \
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
                  walkthrough.cpp

# Qt tools needed to link moc
SOURCES += $$QT_SOURCE_TREE/src/core/tools/qbytearray.cpp       \
                  $$QT_SOURCE_TREE/src/core/tools/qbytearraymatcher.cpp \
                  $$QT_SOURCE_TREE/src/core/tools/qdatetime.cpp \
                  $$QT_SOURCE_TREE/src/core/io/qfile.cpp                \
                  $$QT_SOURCE_TREE/src/core/io/qfileengine.cpp  \
                  $$QT_SOURCE_TREE/src/core/io/qdir.cpp         \
                  $$QT_SOURCE_TREE/src/core/io/qfileinfo.cpp    \
                  $$QT_SOURCE_TREE/src/core/io/qtemporaryfile.cpp \
                  $$QT_SOURCE_TREE/src/core/global/qglobal.cpp          \
                  $$QT_SOURCE_TREE/src/core/io/qiodevice.cpp    \
                  $$QT_SOURCE_TREE/src/core/tools/qlist.cpp             \
                  $$QT_SOURCE_TREE/src/core/tools/qchar.cpp             \
                  $$QT_SOURCE_TREE/src/core/tools/qstring.cpp           \
                  $$QT_SOURCE_TREE/src/core/tools/qstringmatcher.cpp \
                  $$QT_SOURCE_TREE/src/core/tools/qunicodetables.cpp    \
                  $$QT_SOURCE_TREE/src/core/tools/qstringlist.cpp       \
                  $$QT_SOURCE_TREE/src/core/tools/qmap.cpp              \
                  $$QT_SOURCE_TREE/src/core/tools/qvector.cpp          \
                  $$QT_SOURCE_TREE/src/core/tools/qlocale.cpp

SOURCES += $$QT_SOURCE_TREE/src/core/tools/qregexp.cpp \
                  $$QT_SOURCE_TREE/src/core/tools/qbitarray.cpp \
                  $$QT_SOURCE_TREE/src/core/tools/qhash.cpp \
                  $$QT_SOURCE_TREE/src/core/tools/qlinkedlist.cpp \
                  $$QT_SOURCE_TREE/src/core/io/qbuffer.cpp \
                  $$QT_SOURCE_TREE/src/core/io/qtextstream.cpp \
                  $$QT_SOURCE_TREE/src/core/io/qdatastream.cpp \
                  $$QT_SOURCE_TREE/src/core/codecs/qtextcodec.cpp \
                  $$QT_SOURCE_TREE/src/core/codecs/qutfcodec.cpp


unix:SOURCES += $$QT_SOURCE_TREE/src/core/io/qfileengine_unix.cpp

win32:SOURCES += $$QT_SOURCE_TREE/src/core/io/qfileengine_win.cpp

macx: {
SOURCES += $$QT_SOURCE_TREE/src/core/kernel/qcore_mac.cpp
LIBS    += -framework CoreServices
}

mac {
    SOURCES+=$$QT_SOURCE_TREE/src/3rdparty/dlcompat/dlfcn.c
    INCLUDEPATH+=$$QT_SOURCE_TREE/src/3rdparty/dlcompat
    LIBS += -framework CoreFoundation -framework CoreServices
}

include($$QT_SOURCE_TREE/src/core/arch/$$ARCH/arch.pri)

TARGET          = qdoc

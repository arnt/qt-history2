TEMPLATE        = app
TARGET          = idc

CONFIG         += console warn_off qtinc
CONFIG	       -= qt
DESTDIR         = ../../../bin

DEFINES        += QT_NO_THREAD QT_LITE_UNICODE QT_NO_CODECS QT_NO_LIBRARY QT_NO_STL QT_NODLL QT_NO_DATASTREAM \
                  QT_NO_REGEXP QT_NO_COMPRESS QT_NO_TEXTCODEC QT_NO_UNICODETABLES QT_NO_QOBJECT QT_NO_TEXTSTREAM QT_BOOTSTRAPPED
                  
INCLUDEPATH     = ../../corelib/arch/generic $$QT_BUILD_TREE/include $$QT_BUILD_TREE/include/QtCore
DEPENDPATH      += $$INCLUDEPATH ../../corelib/base ../../corelib/tools ../../corelib/io

SOURCES         = main.cpp

SOURCES		+= ../../../../src/corelib/global/qglobal.cpp \
               ../../../../src/corelib/tools/qbytearray.cpp \
               ../../../../src/corelib/tools/qbytearraymatcher.cpp \
               ../../../../src/corelib/tools/qchar.cpp \
               ../../../../src/corelib/tools/qdatetime.cpp \
               ../../../../src/corelib/tools/qhash.cpp \
               ../../../../src/corelib/tools/qlistdata.cpp \
               ../../../../src/corelib/tools/qlocale.cpp \
               ../../../../src/corelib/tools/qstring.cpp \
               ../../../../src/corelib/tools/qstringlist.cpp \
               ../../../../src/corelib/tools/qstringmatcher.cpp \
               ../../../../src/corelib/tools/qunicodetables.cpp \
               ../../../../src/corelib/tools/qvector.cpp \
               ../../../../src/corelib/tools/qvsnprintf.cpp \
               ../../../../src/corelib/kernel/qinternal.cpp

SOURCES		+= \
               ../../../../src/corelib/io/qdir.cpp \
               ../../../../src/corelib/io/qfile.cpp \
               ../../../../src/corelib/io/qfileinfo.cpp \
               ../../../../src/corelib/io/qabstractfileengine.cpp \
               ../../../../src/corelib/io/qfsfileengine.cpp \
               ../../../../src/corelib/io/qiodevice.cpp \
               ../../../../src/corelib/io/qtemporaryfile.cpp

win32:SOURCES   += ../../../../src/corelib/io/qfsfileengine_win.cpp

unix:SOURCES    += ../../../../src/corelib/io/qfsfileengine_unix.cpp

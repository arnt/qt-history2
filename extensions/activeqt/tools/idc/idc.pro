TEMPLATE        = app
TARGET          = idc

CONFIG         += console warn_off qtinc
QT =
DESTDIR         = ../../../../bin

DEFINES        += QT_NO_THREAD QT_LITE_UNICODE QT_NO_CODECS QT_NO_COMPONENT QT_NO_STL QT_NODLL QT_NO_DATASTREAM \
                  QT_NO_REGEXP QT_NO_COMPRESS QT_NO_TEXTCODEC QT_NO_UNICODETABLES QT_NO_QOBJECT QT_NO_TEXTSTREAM
                  
INCLUDEPATH     = ../../../../include/qtcore .
DEPENDPATH      = ../../../../include .

SOURCES         = main.cpp

SOURCES		+= ../../../../src/core/global/qglobal.cpp \
               ../../../../src/core/tools/qbytearray.cpp \
               ../../../../src/core/tools/qbytearraymatcher.cpp \
               ../../../../src/core/tools/qchar.cpp \
               ../../../../src/core/tools/qdatetime.cpp \
               ../../../../src/core/tools/qhash.cpp \
               ../../../../src/core/tools/qlist.cpp \
               ../../../../src/core/tools/qlocale.cpp \
               ../../../../src/core/tools/qstring.cpp \
               ../../../../src/core/tools/qstringlist.cpp \
               ../../../../src/core/tools/qstringmatcher.cpp \
               ../../../../src/core/tools/qunicodetables.cpp \
               ../../../../src/core/tools/qvector.cpp \
               ../../../../src/core/tools/qvsnprintf.cpp \
               ../../../../src/core/kernel/qinternal.cpp

SOURCES		+= ../../../../src/core/io/qbufferedfsfileengine.cpp \
               ../../../../src/core/io/qdir.cpp \
               ../../../../src/core/io/qfile.cpp \
               ../../../../src/core/io/qfileinfo.cpp \
               ../../../../src/core/io/qfileengine.cpp \
               ../../../../src/core/io/qfsfileengine.cpp \
               ../../../../src/core/io/qiodevice.cpp \
               ../../../../src/core/io/qtemporaryfile.cpp

win32:SOURCES   += ../../../../src/core/io/qfsfileengine_win.cpp

unix:SOURCES    += ../../../../src/core/io/qfsfileengine_unix.cpp

TEMPLATE        = app
TARGET          = idc

CONFIG         += console release warn_off qtinc
CONFIG	       -= qt
DESTDIR         = ../../../../bin

DEFINES        += QT_NO_CODECS QT_NO_COMPONENT QT_NO_STL QT_NODLL QT_NO_DATASTREAM QT_NO_REGEXP \
                  QT_NO_COMPRESS QT_NO_SPRINTF QT_NO_TEXTCODEC QT_NO_UNICODETABLES QT_NO_QOBJECT
INCLUDEPATH     = ../../../../include/qtcore .
DEPENDPATH      = ../../../../include .

SOURCES         = main.cpp

SOURCES		+= ../../../../src/core/global/qglobal.cpp \
		   ../../../../src/core/tools/qbytearray.cpp \
                   ../../../../src/core/tools/qbytearraymatcher.cpp \
		   ../../../../src/core/tools/qchar.cpp \
                   ../../../../src/core/tools/qdatetime.cpp \
		   ../../../../src/core/tools/qlist.cpp \
		   ../../../../src/core/tools/qlocale.cpp \
                   ../../../../src/core/tools/qstring.cpp \
		   ../../../../src/core/tools/qstringlist.cpp \
		   ../../../../src/core/tools/qstringmatcher.cpp \
		   ../../../../src/core/tools/qunicodetables.cpp \
                   ../../../../src/core/tools/qvector.cpp

SOURCES		+= ../../../../src/core/io/qdir.cpp \
                   ../../../../src/core/io/qfile.cpp \
                   ../../../../src/core/io/qfileinfo.cpp \
                   ../../../../src/core/io/qfileengine.cpp \
                   ../../../../src/core/io/qiodevice.cpp \
                   ../../../../src/core/io/qtemporaryfile.cpp

win32:SOURCES   += ../../../../src/core/io/qfileengine_win.cpp

unix:SOURCES    += ../../../../src/core/io/qfileengine_unix.cpp

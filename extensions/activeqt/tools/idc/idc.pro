TEMPLATE        = app
TARGET          = idc

CONFIG         += console release warn_off qtinc
CONFIG	       -= qt
DESTDIR         = $$QT_BUILD_TREE\bin

DEFINES        += QT_NO_CODECS QT_NO_COMPONENT QT_NO_STL QT_NODLL QT_NO_DATASTREAM QT_NO_REGEXP QT_NO_COMPRESS QT_NO_SPRINTF QT_NO_TEXTCODEC QT_NO_UNICODETABLES
INCLUDEPATH     = $$QT_SOURCE_TREE/include .
DEPENDPATH      = $$QT_SOURCE_TREE/include .

SOURCES         = main.cpp \
                    $$QT_SOURCE_TREE/src/core/global/qglobal.cpp      \
                    $$QT_SOURCE_TREE/src/core/io/qfile.cpp \
                    $$QT_SOURCE_TREE/src/core/io/qfile_win.cpp \
                    $$QT_SOURCE_TREE/src/core/io/qiodevice.cpp    \
                    $$QT_SOURCE_TREE/src/core/tools/qbytearray.cpp \
		    $$QT_SOURCE_TREE/src/core/tools/qchar.cpp \
		    $$QT_SOURCE_TREE/src/core/tools/qlist.cpp \
		    $$QT_SOURCE_TREE/src/core/tools/qlocale.cpp \
                    $$QT_SOURCE_TREE/src/core/tools/qstring.cpp \
		    $$QT_SOURCE_TREE/src/core/tools/qstringlist.cpp \
		    $$QT_SOURCE_TREE/src/core/tools/qunicodetables.cpp

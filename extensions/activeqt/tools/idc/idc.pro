TEMPLATE        = app
TARGET          = idc

CONFIG         += console release warn_off qtinc
CONFIG	       -= qt
DESTDIR         = $$QT_BUILD_TREE\bin

DEFINES        += QT_NO_CODECS QT_NO_COMPONENT QT_NO_STL QT_NODLL QT_NO_DATASTREAM QT_NO_REGEXP QT_NO_COMPRESS QT_NO_SPRINTF QT_NO_TEXTCODEC QT_NO_UNICODETABLES
INCLUDEPATH     = $$QT_SOURCE_TREE/include .
DEPENDPATH      = $$QT_SOURCE_TREE/include .

SOURCES         = main.cpp \
                    $$QT_SOURCE_TREE/src/tools/qbytearray.cpp \
		    $$QT_SOURCE_TREE/src/tools/qchar.cpp \
                    $$QT_SOURCE_TREE/src/tools/qfile.cpp \
                    $$QT_SOURCE_TREE/src/tools/qfile_win.cpp \
                    $$QT_SOURCE_TREE/src/tools/qglobal.cpp      \
                    $$QT_SOURCE_TREE/src/tools/qiodevice.cpp    \
		    $$QT_SOURCE_TREE/src/tools/qlist.cpp \
		    $$QT_SOURCE_TREE/src/tools/qlocale.cpp \
                    $$QT_SOURCE_TREE/src/tools/qstring.cpp \
		    $$QT_SOURCE_TREE/src/tools/qstringlist.cpp \
		    $$QT_SOURCE_TREE/src/tools/qunicodetables.cpp

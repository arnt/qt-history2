# Project ID used by some IDEs
GUID 		= {6619b429-4254-42d5-a917-6f42b0708bc4}
TEMPLATE        = app
TARGET          = idc

CONFIG         += console release warn_off qtinc
CONFIG	       -= qt
DESTDIR         = $$QT_BUILD_TREE\bin

DEFINES        += QT_NO_CODECS QT_NO_COMPONENT QT_NO_STL QT_NODLL QT_NO_DATASTREAM QT_NO_REGEXP QT_NO_COMPRESS QT_NO_SPRINTF QT_NO_TEXTCODEC QT_NO_UNICODETABLES
INCLUDEPATH     = $$QT_SOURCE_TREE/include .
DEPENDPATH      = $$QT_SOURCE_TREE/include .

SOURCES         = main.cpp \
                    $$QT_SOURCE_TREE/src/tools/qcstring.cpp \
                    $$QT_SOURCE_TREE/src/tools/qfile.cpp \
                    $$QT_SOURCE_TREE/src/tools/qfile_win.cpp \
                    $$QT_SOURCE_TREE/src/tools/qgarray.cpp \
                    $$QT_SOURCE_TREE/src/tools/qgdict.cpp       \
                    $$QT_SOURCE_TREE/src/tools/qglist.cpp       \
                    $$QT_SOURCE_TREE/src/tools/qglobal.cpp      \
                    $$QT_SOURCE_TREE/src/tools/qgvector.cpp     \
                    $$QT_SOURCE_TREE/src/tools/qiodevice.cpp    \
		    $$QT_SOURCE_TREE/src/tools/qlocale.cpp \
                    $$QT_SOURCE_TREE/src/tools/qptrcollection.cpp \
                    $$QT_SOURCE_TREE/src/tools/qstring.cpp \
		    $$QT_SOURCE_TREE/src/tools/qstringlist.cpp \
		    $$QT_SOURCE_TREE/src/tools/qunicodetables.cpp

TEMPLATE        = app
CONFIG          += console release warn_off qtinc
CONFIG		-= qt
DEFINES         += QT_NO_CODECS QT_LITE_UNICODE QT_NO_COMPONENT QT_NO_STL QT_NODLL QT_NO_DATASTREAM QT_NO_REGEXP QT_NO_COMPRESS QT_NO_SPRINTF QT_NO_TEXTCODEC
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
                    $$QT_SOURCE_TREE/src/tools/qptrcollection.cpp \
                    $$QT_SOURCE_TREE/src/tools/qstring.cpp



TARGET          = idc
DESTDIR         = $$QT_BUILD_TREE\bin

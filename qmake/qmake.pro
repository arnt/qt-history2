#This is a project file for building qmake, of course it presents a problem -
# it is very hard to make qmake build this, when qmake is the thing it builds,
#once you are boot strapped though, the qmake.pro will offer better coverage of a
#platform than either of the generic makefiles

CONFIG += console bootstrap
CONFIG -= qt shared app_bundle uic
DEFINES += QT_BUILD_QMAKE
DESTDIR = ../bin/

OBJECTS_DIR = .
MOC_DIR = .

#guts
VPATH += $$QT_SOURCE_TREE/src/core/global \
         $$QT_SOURCE_TREE/src/core/tools \
         $$QT_SOURCE_TREE/src/core/kernel \
         $$QT_SOURCE_TREE/src/core/plugin \
	 $$QT_SOURCE_TREE/src/core/io 
INCPATH += $$QT_SOURCE_TREE/src/core/arch/generic \
           generators generators/unix generators/win32 generators/mac \
           $$QT_SOURCE_TREE/include $$QT_SOURCE_TREE/include/QtCore \
           $$QT_SOURCE_TREE/qmake
include(qmake.pri)


#This is a project file for building qmake, of course it presents a problem -
# it is very hard to make qmake build this, when qmake is the thing it builds,
#once you are boot strapped though, the qmake.pro will offer better coverage of a
#platform than either of the generic makefiles

CONFIG += console
CONFIG -= qt shared resource_fork uic
DESTDIR = ../bin/

OBJECTS_DIR = .
MOC_DIR = .

#guts
include(qmake.pri)

#installation
target.path=$$bins.path
INSTALLS        += target

mkspecs.path=$$data.path/mkspecs
mkspecs.files=$$QT_SOURCE_TREE/mkspecs
INSTALLS        += mkspecs


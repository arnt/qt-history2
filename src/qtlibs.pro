TEMPLATE = subdirs
CONFIG += ordered

#must be built absolutely first!
!win32:!qt_one_lib:SUBDIRS += qnetwork.pro
!qt_one_lib:SUBDIRS += qtkernel.pro 

#must be built second!
SUBDIRS += qt_gui.pro

#can be built in any order..
win32:SUBDIRS += qtmain.pro 
!qt_one_lib:SUBDIRS += qsqlkernel.pro qopengl.pro qxml.pro
embedded:SUBDIRS -= qopengl.pro
SUBDIRS += qcompat.pro 

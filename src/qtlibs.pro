TEMPLATE = subdirs
CONFIG += ordered

!qt_one_lib:SUBDIRS += qtkernel.pro qxml.pro
SUBDIRS += qt_gui.pro
win32:SUBDIRS += qtmain.pro
!qt_one_lib:SUBDIRS += qcompat.pro qnetwork.pro qopengl.pro qsqlkernel.pro
 

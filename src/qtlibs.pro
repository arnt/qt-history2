TEMPLATE = subdirs
CONFIG += ordered

!qt_one_lib { 
    SUBDIRS += qtkernel.pro
    SUBDIRS += qxml.pro qsqlkernel.pro #hacks to satisfy qt.pro - see hacks comment with --Sam
}
SUBDIRS += qt_gui.pro
win32:SUBDIRS += qtmain.pro
!qt_one_lib:SUBDIRS += qcompat.pro qnetwork.pro qopengl.pro 
 

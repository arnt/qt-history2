TEMPLATE = subdirs
CONFIG += ordered

#must be built absolutely first!
!qt_one_lib:SUBDIRS += qnetwork.pro qtkernel.pro 

#must be built second!
SUBDIRS += qt_gui.pro

#can be built in any order..
win32:SUBDIRS += qtmain.pro 
!qt_one_lib:SUBDIRS += qsqlkernel.pro qopengl.pro qxml.pro
embedded:SUBDIRS -= qopengl.pro
#once all of the stuff in compat is out of Qt (now linked into qtkernel) we can uncomment this --SAM
#SUBDIRS += qcompat.pro 

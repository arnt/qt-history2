TEMPLATE = subdirs
CONFIG += ordered

#must be built absolutely first!
!qt_one_lib:SUBDIRS += qcore.pro 

#must be built second!
SUBDIRS += qgui.pro

#can be built in any order..
win32:SUBDIRS += qtmain.pro 
!qt_one_lib {
   SUBDIRS += qnetwork.pro qsql.pro qopengl.pro qxml.pro
}
embedded:SUBDIRS -= qopengl.pro
SUBDIRS += qcompat.pro 

TEMPLATE = subdirs
CONFIG += ordered

# this order is important
win32:SUBDIRS += winmain
SUBDIRS += moc core gui opengl sql 
!contains(QT_PRODUCT, qt-professional):SUBDIRS += xml network canvas
SUBDIRS += compat
SUBDIRS += plugins

embedded:SUBDIRS -= qopengl.pro

TEMPLATE = subdirs
CONFIG += ordered

# this order is important
win32:SUBDIRS += winmain
SUBDIRS += moc core gui opengl sql compat plugins
!contains(QT_PRODUCT, qt-professional):SUBDIRS += xml network canvas

embedded:SUBDIRS -= qopengl.pro

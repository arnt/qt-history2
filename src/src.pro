TEMPLATE = subdirs
CONFIG += ordered

# this order is important
win32:SUBDIRS += winmain
SUBDIRS += moc core gui canvas network opengl sql xml compat plugins

#can be built in any order..
win32:SUBDIRS -= moc

embedded:SUBDIRS -= qopengl.pro

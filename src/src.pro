TEMPLATE = subdirs
CONFIG += ordered

# this order is important
SUBDIRS = moc core gui network opengl sql xml compat


#can be built in any order..
win32:SUBDIRS += qtmain.pro 
win32:SUBDIRS -= moc

embedded:SUBDIRS -= qopengl.pro

TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += archive \
           package

win:SUBDIRS += win
mac:SUBDIRS += mac

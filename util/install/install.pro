TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += archive \
           package \
	   keygen

win:SUBDIRS += win
mac:SUBDIRS += mac

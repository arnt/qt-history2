GUID 	 = {e7ede3f4-a04c-4426-8f01-8b241b965f9b}
TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += archive \
           package \
	   keygen

win32:SUBDIRS += win
mac:SUBDIRS += mac

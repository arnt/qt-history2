TEMPLATE	= subdirs
win32:SUBDIRS	= designer \
		  assistant \
		  linguist
unix:SUBDIRS	= designer \
		  assistant \
		  linguist \
		  qtconfig
CONFIG+=ordered
REQUIRES=full-config nocrosscompiler

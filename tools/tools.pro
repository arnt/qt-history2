TEMPLATE	= subdirs
win32:SUBDIRS	= assistant/lib \
		  designer \
		  assistant \
		  linguist
unix:SUBDIRS	= assistant/lib \
		  designer \
		  assistant \
		  linguist \
		  qtconfig
CONFIG+=ordered
REQUIRES=full-config nocrosscompiler

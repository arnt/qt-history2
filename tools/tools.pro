TEMPLATE	= subdirs
SUBDIRS		= assistant/lib \
		  designer \
		  assistant \
		  linguist
unix:SUBDIRS	+= qtconfig

CONFIG+=ordered
REQUIRES=full-config nocrosscompiler

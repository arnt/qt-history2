TEMPLATE	= subdirs
SUBDIRS		= assistant/lib \
		  designer \
		  assistant \
		  linguist
unix:SUBDIRS	+= qtconfig

CONFIG+=ordered
QTDIR_build:REQUIRES=full-config nocrosscompiler

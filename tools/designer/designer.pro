TEMPLATE	= subdirs
SUBDIRS		=  uic \
		   uilib \
		   designer 
dll:SUBDIRS *=  editor plugins
shared:SUBDIRS *=  editor plugins
CONFIG += ordered

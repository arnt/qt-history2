TEMPLATE	= subdirs
SUBDIRS		=  uic \
		   uilib \
		   designer \
		   designer/app
dll:SUBDIRS *=  editor plugins
shared:SUBDIRS *=  editor plugins
CONFIG += ordered

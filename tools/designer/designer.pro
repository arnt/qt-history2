TEMPLATE	= subdirs
SUBDIRS		=  uic \
		   uilib \
		   designer \
		   editor
dll:SUBDIRS	+=  plugins/wizards \
		  plugins/cppeditor \
		  plugins/dlg \
		  plugins/rc
CONFIG += ordered


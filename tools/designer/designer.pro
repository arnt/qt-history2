TEMPLATE	= subdirs
SUBDIRS		=  uic \
		   uilib \
		   designer		   
shared {
    SUBDIRS 	*= \
		editor \
		plugins/wizards \
		plugins/cppeditor \
		plugins/dlg \
		plugins/rc
}
dll {
    SUBDIRS 	*= \
		editor \
		plugins/wizards \
		plugins/cppeditor \
		plugins/dlg \
		plugins/rc
}

CONFIG += ordered

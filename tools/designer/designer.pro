TEMPLATE	= subdirs
SUBDIRS		=  uic \
		   uilib \
		   designer		   
win32 {
    contains(DEFINES,QT_DLL):SUBDIRS += \
		editor \
		plugins/wizards \
		plugins/cppeditor \
		plugins/dlg \
		plugins/rc
}
unix {
    dll:SUBDIRS	+= \
		editor \
		plugins/wizards \
		plugins/cppeditor \
		plugins/dlg \
		plugins/rc
}

CONFIG += ordered

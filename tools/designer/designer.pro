TEMPLATE	= subdirs
SUBDIRS		=  uic \
		   uilib \
		   designer \
		   editor
win32 {
    contains(DEFINES,QT_DLL):SUBDIRS += \
		plugins/wizards \
		plugins/cppeditor \
		plugins/dlg \
		plugins/rc
}
unix {
    dll:SUBDIRS	+= \
		plugins/wizards \
		plugins/cppeditor \
		plugins/dlg \
		plugins/rc
}

CONFIG += ordered

# Qt core library and plugin module

HEADERS += \
	library/qcom_p.h \
	library/qcomlibrary_p.h \
	library/qgpluginmanager_p.h \
	library/qlibrary.h \
	library/qlibrary_p.h \
	library/qpluginmanager_p.h \
	library/qgplugin.h \
	library/quuid.h

SOURCES += \
	library/qcomlibrary.cpp \
	library/qgpluginmanager.cpp \
	library/qgplugin.cpp \
	library/quuid.cpp

win32 {
	SOURCES += library/qlibrary_win.cpp
}

mac { 
	SOURCES+=3rdparty/dlcompat/dlfcn.c
	INCLUDEPATH+=3rdparty/dlcompat
}

unix {
	SOURCES += library/qlibrary_unix.cpp
}

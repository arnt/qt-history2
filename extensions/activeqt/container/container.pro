TEMPLATE        = lib
CONFIG          += qt warn_on

internal {
	ACTIVEQT_H  = .
}
!internal {
	ACTIVEQT_H  = $$QT_SOURCE_TREE/include
}

HEADERS         = $$ACTIVEQT_H/qaxbase.h \
		  $$ACTIVEQT_H/qaxwidget.h \
		  $$ACTIVEQT_H/qaxobject.h \
		  ../shared/types.h

SOURCES         = qaxbase.cpp \
		  qaxwidget.cpp \
		  qaxobject.cpp \
		  ../shared/types.cpp

FORMS           = qactivexselect.ui

DESTDIR         = $$QT_BUILD_TREE/lib
DLLDESTDIR      = $$QT_BUILD_TREE/bin
TARGET          = qaxcontainer
INCLUDEPATH     += $$QT_SOURCE_TREE/tools/designer/interfaces

shared {
    CONFIG      += plugin
    SOURCES     += plugin.cpp
    DLLDESTDIR  += $$QT_BUILD_TREE/plugins/designer
}

win32-borland:INCLUDEPATH += $(BCB)/include/Atl

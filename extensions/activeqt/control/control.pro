TEMPLATE    = lib
CONFIG      += qt warn_off staticlib
TARGET      = qaxserver

internal {
	ACTIVEQT_H  = .
}
!internal {
	ACTIVEQT_H  = $$QT_SOURCE_TREE/include
}

HEADERS     = $$ACTIVEQT_H/qaxbindable.h \
	      $$ACTIVEQT_H/qaxfactory.h \
	      ../shared/types.h

SOURCES     = qaxserverbase.cpp \
	      qaxbindable.cpp \
	      qaxfactory.cpp \
	      qaxservermain.cpp \
	      qaxserverdll.cpp \
	      ../shared/types.cpp

DESTDIR     = $$QT_BUILD_TREE\lib

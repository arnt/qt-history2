TEMPLATE	= app
CONFIG		+= qt console warn_on release professional
HEADERS	= uic.h \
		  ../shared/widgetdatabase.h \
		  ../shared/domtool.h \
		  ../integration/kdevelop/kdewidgets.h \
		  ../shared/parser.h \
		  ../interfaces/widgetinterface.h

SOURCES	= uic.cpp form.cpp object.cpp \
		  ../shared/widgetdatabase.cpp  \
		  ../shared/domtool.cpp \
		  ../shared/parser.cpp \
		  ../integration/kdevelop/kdewidgets.cpp

include( ../../../src/qt_professional.pri )

TARGET		= uic
INCLUDEPATH	+= ../shared ../../../src/3rdparty/zlib/
!zlib:unix:LIBS	+= -lz
DEFINES 	+= UIC
DESTDIR		= ../../../bin

target.path=$$QT_INSTALL_BINPATH
isEmpty(target.path):target.path=$$QT_PREFIX/bin
INSTALLS        += target

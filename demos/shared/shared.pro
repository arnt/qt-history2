TEMPLATE = lib
CONFIG += static
build_all:CONFIG += release
TARGET = demo_shared

SOURCES += \
	arthurstyle.cpp\
	arthurwidgets.cpp \
	hoverpoints.cpp

HEADERS += \
	arthurstyle.h \
	arthurwidgets.h \
	hoverpoints.h 

RESOURCES += shared.qrc

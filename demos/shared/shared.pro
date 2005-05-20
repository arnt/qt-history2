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

# install
target.path = $$[QT_INSTALL_DATA]/demos/shared
sources.files = $$SOURCES $$HEADERS $$RESOURCES *.pro *.pri images
sources.path = $$[QT_INSTALL_DATA]/demos/shared
INSTALLS += target sources

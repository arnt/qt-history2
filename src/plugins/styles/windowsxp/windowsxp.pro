TARGET	 = qwindowsxpstyle
include(../../qpluginbase.pri)

DESTDIR	 = $$QT_BUILD_TREE/plugins/styles

HEADERS		= \
                    ../../../gui/styles/qwindowsxpstyle.h \
                    ../../../gui/styles/qwindowsstyle.h
SOURCES		= \
                    main.cpp \
		    ../../../gui/styles/qwindowsxpstyle.cpp \
                    ../../../gui/styles/qwindowsstyle.cpp

target.path += $$[QT_INSTALL_PLUGINS]/styles
INSTALLS += target

LIBS += gdi32.lib user32.lib
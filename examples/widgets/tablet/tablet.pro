HEADERS	    =	mainwindow.h \
		tabletcanvas.h \
		tabletapplication.h
SOURCES	    =	mainwindow.cpp \
		main.cpp \
		tabletcanvas.cpp \
		tabletapplication.cpp

# install
target.path = $$[QT_INSTALL_EXAMPLES]/widgets/tablet
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS tablet.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/widgets/tablet
INSTALLS += target sources

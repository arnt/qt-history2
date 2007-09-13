HEADERS	    = commands.h \
	      diagramitem.h \
	      diagramscene.h \
	      mainwindow.h
SOURCES	    = commands.cpp \
              diagramitem.cpp \
              diagramscene.cpp \
              main.cpp \
              mainwindow.cpp
RESOURCES   = undoframework.qrc

# install
target.path = $$[QT_INSTALL_EXAMPLES]/tools/undoframework
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS undoframework.pro README
sources.path = $$[QT_INSTALL_EXAMPLES]/tools/undoframework
INSTALLS += target sources
DEFINES += QT_USE_USING_NAMESPACE

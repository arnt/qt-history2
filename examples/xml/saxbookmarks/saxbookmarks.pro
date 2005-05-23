HEADERS       = mainwindow.h \
                xbelgenerator.h \
                xbelhandler.h
SOURCES       = main.cpp \
                mainwindow.cpp \
                xbelgenerator.cpp \
                xbelhandler.cpp
QT           += xml

# install
target.path = $$[QT_INSTALL_DATA]/examples/xml/saxbookmarks
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS saxbookmarks.pro *.xbel
sources.path = $$[QT_INSTALL_DATA]/examples/xml/saxbookmarks
INSTALLS += target sources

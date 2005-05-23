HEADERS       = ../connection.h \
                tableeditor.h
SOURCES       = main.cpp \
                tableeditor.cpp
QT           += sql

# install
target.path = $$[QT_INSTALL_DATA]/examples/sql/cachedtable
sources.files = $$SOURCES *.h $$RESOURCES $$FORMS cachedtable.pro
sources.path = $$[QT_INSTALL_DATA]/examples/sql/cachedtable
INSTALLS += target sources

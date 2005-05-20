HEADERS       = ../connection.h \
                customsqlmodel.h \
                editablesqlmodel.h
SOURCES       = customsqlmodel.cpp \
                editablesqlmodel.cpp \
                main.cpp
QT           += sql

# install
target.path = $$[QT_INSTALL_DATA]/examples/sql/querymodel
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS querymodel.pro
sources.path = $$[QT_INSTALL_DATA]/examples/sql/querymodel
INSTALLS += target sources

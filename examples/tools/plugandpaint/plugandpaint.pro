HEADERS        = interfaces.h \
                 mainwindow.h \
                 paintarea.h \
                 plugindialog.h
SOURCES        = main.cpp \
                 mainwindow.cpp \
                 paintarea.cpp \
                 plugindialog.cpp

# install
target.path = $$[QT_INSTALL_DATA]/examples/tools/plugandpaint
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS plugandpaint.pro
sources.path = $$[QT_INSTALL_DATA]/examples/tools/plugandpaint
INSTALLS += target sources

HEADERS       = tabdialog.h
SOURCES       = main.cpp \
                tabdialog.cpp

# install
target.path = $$[QT_INSTALL_DATA]/examples/dialogs/tabdialog
sources.files = $$SOURCES $$HEADERS *.pro
sources.path = $$[QT_INSTALL_DATA]/examples/dialogs/tabdialog
INSTALLS += target sources

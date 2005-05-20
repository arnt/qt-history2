HEADERS       = dialog.h
SOURCES       = dialog.cpp \
                main.cpp

# install
target.path = $$[QT_INSTALL_DATA]/examples/dialogs/standarddialogs
sources.files = $$SOURCES $$HEADERS *.pro
sources.path = $$[QT_INSTALL_DATA]/examples/dialogs/standarddialogs
INSTALLS += target sources

HEADERS   = mainwindow.h \
            historymodel.h
SOURCES   = historymodel.cpp \
            main.cpp \
            mainwindow.cpp
RESOURCES = completer.qrc

# install
target.path = $$[QT_INSTALL_EXAMPLES]/tools/completer
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS completer.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/tools/completer
INSTALLS += target sources

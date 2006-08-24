HEADERS   = mainwindow.h \
            textedit.h
SOURCES   = main.cpp \
            mainwindow.cpp \
            textedit.cpp
RESOURCES = customcompleter.qrc

# install
target.path = $$[QT_INSTALL_EXAMPLES]/tools/customcompleter
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS customcompleter.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/tools/customcompleter
INSTALLS += target sources

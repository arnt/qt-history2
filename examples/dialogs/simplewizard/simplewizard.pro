HEADERS       = classwizard.h \
                simplewizard.h
SOURCES       = classwizard.cpp \
                main.cpp \
                simplewizard.cpp
# install
target.path = $$[QT_INSTALL_EXAMPLES]/dialogs/simplewizard
sources.files = $$SOURCES $$HEADERS *.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/dialogs/simplewizard
INSTALLS += target sources

HEADERS       = mainwindow.h \
                printpanel.h
SOURCES       = main.cpp \
                mainwindow.cpp \
                printpanel.cpp
TRANSLATIONS  = trollprint_pt.ts

# install
target.path = $$[QT_INSTALL_DATA]/examples/linguist/trollprint
sources.files = $$SOURCES $$HEADERS $$TRANSLATIONS *.pro
sources.path = $$[QT_INSTALL_DATA]/examples/linguist/trollprint
INSTALLS += target sources

HEADERS      = arrowpad.h \
               mainwindow.h
SOURCES      = arrowpad.cpp \
               main.cpp \
               mainwindow.cpp
TRANSLATIONS = arrowpad_fr.ts \
               arrowpad_nl.ts

# install
target.path = $$[QT_INSTALL_DATA]/examples/linguist/arrowpad
sources.files = $$SOURCES $$HEADERS *.pro
sources.path = $$[QT_INSTALL_DATA]/examples/linguist/arrowpad
INSTALLS += target sources

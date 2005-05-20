SOURCES      = main.cpp
TRANSLATIONS = hellotr_la.ts

# install
target.path = $$[QT_INSTALL_DATA]/examples/linguist/hellotr
sources.files = $$SOURCES *.pro
sources.path = $$[QT_INSTALL_DATA]/examples/linguist/hellotr
INSTALLS += target sources

SOURCES       = main.cpp

# install
target.path = $$[QT_INSTALL_DATA]/examples/itemviews/dirview
sources.files = $$SOURCES *.pro
sources.path = $$[QT_INSTALL_DATA]/examples/itemviews/dirview
INSTALLS += target sources

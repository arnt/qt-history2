HEADERS       = imageviewer.h
SOURCES       = imageviewer.cpp \
                main.cpp

# install
target.path = $$[QT_INSTALL_DATA]/examples/widgets/imageviewer
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS imageviewer.pro
sources.path = $$[QT_INSTALL_DATA]/examples/widgets/imageviewer
INSTALLS += target sources

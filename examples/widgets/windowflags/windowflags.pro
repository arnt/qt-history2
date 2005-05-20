HEADERS       = controllerwindow.h \
                previewwindow.h
SOURCES       = controllerwindow.cpp \
                previewwindow.cpp \
                main.cpp

# install
target.path = $$[QT_INSTALL_DATA]/examples/widgets/windowflags
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS windowflags.pro
sources.path = $$[QT_INSTALL_DATA]/examples/widgets/windowflags
INSTALLS += target sources

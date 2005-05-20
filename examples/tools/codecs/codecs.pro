HEADERS      += mainwindow.h \
                previewform.h
SOURCES      += main.cpp \
                mainwindow.cpp \
                previewform.cpp

# install
target.path = $$[QT_INSTALL_DATA]/examples/tools/codecs
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS codecs.pro
sources.path = $$[QT_INSTALL_DATA]/examples/tools/codecs
INSTALLS += target sources

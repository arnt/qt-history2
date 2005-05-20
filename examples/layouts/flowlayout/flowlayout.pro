HEADERS     = flowlayout.h \
              window.h
SOURCES     = flowlayout.cpp \
              main.cpp \
              window.cpp

# install
target.path = $$[QT_INSTALL_DATA]/examples/layouts/flowlayout
sources.files = $$SOURCES $$HEADERS *.pro
sources.path = $$[QT_INSTALL_DATA]/examples/layouts/flowlayout
INSTALLS += target sources

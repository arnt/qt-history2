HEADERS     = characterwidget.h \
              mainwindow.h
SOURCES     = characterwidget.cpp \
              mainwindow.cpp \
              main.cpp

# install
target.path = $$[QT_INSTALL_EXAMPLES]/widgets/charactermap
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS charactermap.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/widgets/charactermap
INSTALLS += target sources

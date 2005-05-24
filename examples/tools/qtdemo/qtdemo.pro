CONFIG        += assistant
DESTDIR       = $$QT_BUILD_TREE/bin
HEADERS       = displayshape.h \
                displaywidget.h \
                launcher.h
QT            += xml
RESOURCES     = qtdemo.qrc
SOURCES       = displayshape.cpp \
                displaywidget.cpp \
                launcher.cpp \
                main.cpp
TARGET        = qtdemo

win32 {
   RC_FILE	= qtdemo.rc
}

# install
target.path = $$[QT_INSTALL_BIN]
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS qtdemo.pro images *.xml
sources.path = $$[QT_INSTALL_DATA]/examples/tools/qtdemo
INSTALLS += target sources

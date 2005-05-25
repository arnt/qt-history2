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

mac {
    ICON = qtdemo.icns
}


# install
target.path = $$[QT_INSTALL_BINS]
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS qtdemo.pro images *.xml *.ico *.icns *.rc
sources.path = $$[QT_INSTALL_EXAMPLES]/tools/qtdemo
INSTALLS += target sources

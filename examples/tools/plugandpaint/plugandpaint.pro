HEADERS        = interfaces.h \
                 mainwindow.h \
                 paintarea.h \
                 plugindialog.h
SOURCES        = main.cpp \
                 mainwindow.cpp \
                 paintarea.cpp \
                 plugindialog.cpp
LIBS           = -Lplugins -lpnp_basictools

CONFIG(debug, debug|release) {
   unix:LIBS = $$member(LIBS, 0) $$member(LIBS, 1)_debug
   else:LIBS = $$member(LIBS, 0) $$member(LIBS, 1)d
}

# install
target.path = $$[QT_INSTALL_EXAMPLES]/tools/plugandpaint
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS plugandpaint.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/tools/plugandpaint
INSTALLS += target sources

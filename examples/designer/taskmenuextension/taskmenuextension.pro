TEMPLATE = lib
TARGET   = $$qtLibraryTarget($$TARGET)
CONFIG  += designer plugin debug_and_release
DESTDIR  = $$QT_BUILD_TREE/plugins/designer

HEADERS += tictactoe.h \
           tictactoedialog.h \
           tictactoeplugin.h \
           tictactoetaskmenu.h
SOURCES += tictactoe.cpp \
           tictactoedialog.cpp \
           tictactoeplugin.cpp \
           tictactoetaskmenu.cpp

# install
target.path = $$[QT_INSTALL_PLUGINS]/designer
sources.files = $$SOURCES $$HEADERS *.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/designer/taskmenuextension
INSTALLS += target sources

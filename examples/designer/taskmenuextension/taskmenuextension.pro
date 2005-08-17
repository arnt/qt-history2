TEMPLATE = lib
CONFIG  += designer release plugin
DESTDIR  = $(QTDIR)/plugins/designer

# Input
HEADERS += tictactoe.h \
           tictactoedialog.h \
           tictactoeplugin.h \
           tictactoetaskmenu.h
SOURCES += tictactoe.cpp \
           tictactoedialog.cpp \
           tictactoeplugin.cpp \
           tictactoetaskmenu.cpp

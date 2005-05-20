HEADERS     = mainwindow.h \
              pieceslist.h \
              puzzlewidget.h
RESOURCES   = puzzle.qrc
SOURCES     = main.cpp \
              mainwindow.cpp \
              pieceslist.cpp \
              puzzlewidget.cpp

# install
target.path = $$[QT_INSTALL_DATA]/examples/draganddrop/puzzle
sources.files = $$SOURCES $$HEADERS $$RESOURCES *.pro *.jpg
sources.path = $$[QT_INSTALL_DATA]/examples/draganddrop/puzzle
INSTALLS += target sources

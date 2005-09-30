HEADERS     = mainwindow.h \
              piecesmodel.h \
              puzzlewidget.h
RESOURCES   = puzzle.qrc
SOURCES     = main.cpp \
              mainwindow.cpp \
              piecesmodel.cpp \
              puzzlewidget.cpp

# install
target.path = $$[QT_INSTALL_EXAMPLES]/draganddrop/puzzle
sources.files = $$SOURCES $$HEADERS $$RESOURCES *.pro *.jpg
sources.path = $$[QT_INSTALL_EXAMPLES]/draganddrop/puzzle
INSTALLS += target sources

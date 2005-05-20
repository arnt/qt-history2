HEADERS         = highlighter.h \
                  mainwindow.h
RESOURCES       = syntaxhighlighter.qrc
SOURCES         = highlighter.cpp \
                  mainwindow.cpp \
                  main.cpp

# install
target.path = $$[QT_INSTALL_DATA]/examples/richtext/syntaxhighlighter
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS syntaxhighlighter.pro
sources.path = $$[QT_INSTALL_DATA]/examples/richtext/syntaxhighlighter
INSTALLS += target sources

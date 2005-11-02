HEADERS     = treeitem.h \
              treemodel.h
RESOURCES   = sortingmodel.qrc
SOURCES     = main.cpp \
              treeitem.cpp \
              treemodel.cpp

# install
target.path = $$[QT_INSTALL_EXAMPLES]/itemviews/sortingmodel
sources.files = $$SOURCES $$HEADERS $$RESOURCES *.pro *.txt
sources.path = $$[QT_INSTALL_EXAMPLES]/itemviews/sortingmodel
INSTALLS += target sources

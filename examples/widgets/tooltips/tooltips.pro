HEADERS       = shapeitem.h \
                sortingbox.h
SOURCES       = main.cpp \
                shapeItem.cpp \
                sortingbox.cpp
RESOURCES     = tooltips.qrc

# install
target.path = $$[QT_INSTALL_DATA]/examples/widgets/tooltips
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS tooltips.pro images
sources.path = $$[QT_INSTALL_DATA]/examples/widgets/tooltips
INSTALLS += target sources

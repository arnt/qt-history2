HEADERS     = detailsdialog.h \
              mainwindow.h
SOURCES     = detailsdialog.cpp \
              main.cpp \
              mainwindow.cpp

# install
target.path = $$[QT_INSTALL_DATA]/examples/richtext/orderform
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS orderform.pro
sources.path = $$[QT_INSTALL_DATA]/examples/richtext/orderform
INSTALLS += target sources

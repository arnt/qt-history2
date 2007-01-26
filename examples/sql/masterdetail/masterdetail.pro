HEADERS   = database.h \
            dialog.h \
            mainwindow.h
SOURCES   = dialog.cpp \
            main.cpp \
            mainwindow.cpp

QT += sql
QT += xml

# install
target.path = $$[QT_INSTALL_EXAMPLES]/sql/masterdetail
sources.files = $$SOURCES *.h $$RESOURCES $$FORMS masterdetail.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/sql/masterdetail
INSTALLS += target sources

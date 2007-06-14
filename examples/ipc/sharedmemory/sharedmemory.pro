SOURCES += main.cpp \
           dialog.cpp 

HEADERS += dialog.h

# Forms and resources
FORMS += dialog.ui
#RESOURCES += icons.qrc

# install
target.path = $$[QT_INSTALL_EXAMPLES]/ipc/sharedmemory
sources.files = $$SOURCES $$HEADERS $$RESOURCES ipc.pro *.png
sources.path = $$[QT_INSTALL_EXAMPLES]/ipc/sharedmemory
INSTALLS += target sources

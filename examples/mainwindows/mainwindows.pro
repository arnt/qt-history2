TEMPLATE      = subdirs
SUBDIRS       = application \
                mdi \
                menus \
                recentfiles \
                sdi

# install
target.path = $$[QT_INSTALL_DATA]/examples/mainwindows
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS mainwindows.pro README
sources.path = $$[QT_INSTALL_DATA]/examples/mainwindows
INSTALLS += target sources

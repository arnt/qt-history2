TEMPLATE      = subdirs
SUBDIRS       = dialogs \
                draganddrop \
                itemviews \
                layouts \
                linguist \
                mainwindows \
                network \
                painting \
                qtestlib \
                richtext \
                sql \
                threads \
                tools \
                tutorial \
                widgets \
                xml
!contains(QT_EDITION, Console):SUBDIRS += designer
contains(QT_CONFIG, opengl): SUBDIRS += opengl
#!cross_compiler:SUBDIRS: += designer
win32:!contains(QT_EDITION, OpenSource|Console):SUBDIRS += activeqt

# install
sources.files = README *.pro
sources.path = $$[QT_INSTALL_EXAMPLES]
INSTALLS += sources

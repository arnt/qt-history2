TEMPLATE      = subdirs
SUBDIRS       = \
                desktop \
                dialogs \
                draganddrop \
                graphicsview \
                itemviews \
                layouts \
                linguist \
                mainwindows \
                network \
                painting \
                richtext \
                sql \
                threads \
                tools \
                tutorial \
                widgets \
                xml \
                script

embedded:SUBDIRS += qtopiacore
!contains(QT_EDITION, Console):contains(QT_BUILD_PARTS, tools):SUBDIRS += designer
contains(QT_BUILD_PARTS, tools):SUBDIRS += assistant qtestlib
contains(QT_CONFIG, opengl): SUBDIRS += opengl
contains(QT_CONFIG, qdbus): SUBDIRS += qdbus
win32:!contains(QT_EDITION, OpenSource|Console):SUBDIRS += activeqt

# install
sources.files = README *.pro
sources.path = $$[QT_INSTALL_EXAMPLES]
INSTALLS += sources

TEMPLATE      = subdirs
SUBDIRS       = assistant \
                dialogs \
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
embedded:SUBDIRS += qtopiacore
!contains(QT_EDITION, Console):!cross_compile:SUBDIRS += designer
contains(QT_CONFIG, opengl): SUBDIRS += opengl
win32:!contains(QT_EDITION, OpenSource|Console):SUBDIRS += activeqt

# install
sources.files = README *.pro
sources.path = $$[QT_INSTALL_EXAMPLES]
INSTALLS += sources

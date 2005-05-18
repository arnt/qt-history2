TEMPLATE      = subdirs
SUBDIRS       = dialogs \
                draganddrop \
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
                xml

contains(QT_CONFIG, opengl): SUBDIRS += opengl
!cross_compiler:SUBDIRS: += designer
win32:SUBDIRS += activeqt

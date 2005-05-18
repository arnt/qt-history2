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

!cross_compiler:SUBDIRS: += designer
contains(QT_CONFIG, opengl): SUBDIRS += opengl

contains(QT_PRODUCT, qt-(enterprise|internal|eval)) {
    win32:SUBDIRS += activeqt
    x11:SUBDIRS += motif
}

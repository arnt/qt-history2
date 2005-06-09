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
                xml \
		designer

contains(QT_CONFIG, opengl): SUBDIRS += opengl
#!cross_compiler:SUBDIRS: += designer
win32:!contains(QT_PRODUCT, .*OpenSource.*|.*Console.*):SUBDIRS += activeqt

# install
sources.files = README *.pro
sources.path = $$[QT_INSTALL_EXAMPLES]
INSTALLS += sources

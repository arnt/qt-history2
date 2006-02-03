TEMPLATE        = subdirs
no-png {
    message("Tools not available without PNG support")
} else {
    SUBDIRS		= assistant/lib \
			assistant \
			linguist \
			porting \
                        qtestlib
    !contains(QT_EDITION, Console):SUBDIRS += designer
    unix:!embedded:contains(QT_CONFIG, qt3support):SUBDIRS += qtconfig
    win32:!contains(QT_EDITION, OpenSource|Console):SUBDIRS += activeqt
}

CONFIG+=ordered
QTDIR_build:REQUIRES = "contains(QT_CONFIG, full-config)"

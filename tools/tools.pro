TEMPLATE        = subdirs
no-png {
    message("Tools not available without PNG support")
} else {
    SUBDIRS		= assistant/lib \
			assistant \
			linguist \
			porting
    !contains(QT_EDITION, Console):SUBDIRS += designer
    unix:SUBDIRS        += qtconfig
    win32:SUBDIRS       += activeqt
}

CONFIG+=ordered
QTDIR_build:REQUIRES = "contains(QT_CONFIG, full-config)"

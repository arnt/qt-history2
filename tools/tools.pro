TEMPLATE        = subdirs
no-png {
    message("Tools not available without PNG support")
} else {
    SUBDIRS		= assistant/lib \
			assistant \
			linguist \
			porting
    !contains(QT_PRODUCT, .*Console.*):SUBDIRS += designer
    unix:SUBDIRS        += qtconfig
}

CONFIG+=ordered
QTDIR_build:REQUIRES = "contains(QT_CONFIG, full-config)"

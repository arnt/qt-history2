TEMPLATE = subdirs

CONFIG	+= ordered

!contains(QT_PRODUCT, qt-(enterprise|internal)) {
    message("ActiveQt requires a Qt/Enterprise license.")
} else {
    SUBDIRS = idc \
              dumpdoc \
              dumpcpp \
              testcon
}

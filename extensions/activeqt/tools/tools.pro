TEMPLATE = subdirs

CONFIG	+= ordered

contains(QT_PRODUCT, .*OpenSource.*|.*Console.*) {
    message("You are not licensed to use ActiveQt.")
} else {
    SUBDIRS = idc \
              dumpdoc \
              dumpcpp \
              testcon
}

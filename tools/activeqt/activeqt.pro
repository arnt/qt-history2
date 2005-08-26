TEMPLATE = subdirs

CONFIG	+= ordered

contains(QT_EDITION, OpenSource|Console) {
    message("You are not licensed to use ActiveQt.")
} else {
    SUBDIRS = idc \
              dumpdoc \
              dumpcpp \
              testcon
}

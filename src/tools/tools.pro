TEMPLATE = subdirs
SUBDIRS = uic rcc
contains(QT_CONFIG, compat): SUBDIRS += uic3


TEMPLATE = subdirs
SUBDIRS = uic rcc
contains(QT_CONFIG, qt3support): SUBDIRS += uic3


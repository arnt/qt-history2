TEMPLATE = subdirs
SUBDIRS = uic rcc
!win32-borland:contains(QT_CONFIG, qt3support): SUBDIRS += uic3


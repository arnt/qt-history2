TEMPLATE = subdirs
SUBDIRS = uic rcc
!win32-borland {
	!win32-g++:contains(QT_CONFIG, qt3support): SUBDIRS += uic3
}


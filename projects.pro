#####################################################################
# Main projectfile
#####################################################################

CONFIG += ordered
TEMPLATE = subdirs
isEmpty(QT_PROJECTS) {
#  QT_PROJECTS = qmake
   QT_PROJECTS += src tools demos examples tutorial
}
SUBDIRS += $$QT_PROJECTS

unix {
  confclean.depends += clean
  confclean.commands += $(DEL_FILE) .qmake.cache \
			&& (cd config.tests/unix/stl && $(MAKE) distclean) \
			&& (cd config.tests/unix/endian && $(MAKE) distclean) \
			&& (cd config.tests/unix/ipv6 && $(MAKE) distclean) \
			&& (cd config.tests/unix/largefile && $(MAKE) distclean) \
			&& (cd config.tests/unix/ptrsize && $(MAKE) distclean) \
			&& (cd config.tests/x11/notype && $(MAKE) distclean)
  QMAKE_EXTRA_UNIX_TARGETS += confclean
}
CONFIG -= qt

### installations ####

#docs
htmldocs.files = $$QT_BUILD_TREE/doc/html/*
htmldocs.path = $$[QT_INSTALL_DOCS]/html
INSTALLS += htmldocs

#translations
translations.path=$$[QT_INSTALL_TRANSLATIONS]
translations.files = $$QT_BUILD_TREE/translations/*.qm
INSTALLS += translations

#qt.conf
!isEmpty(QT_INSTALL_QTCONFIG) {
    qt_conf.path=$$QT_INSTALL_QTCONFIG
    qt_conf.files = $$QT_BUILD_TREE/qt.conf
    INSTALLS += qt_conf
}

#qmake
qmake.path=$$[QT_INSTALL_BINS]
qmake.files=$$QT_BUILD_TREE/bin/qmake #exe itself
INSTALLS += qmake

#mkspecs
mkspecs.path=$$[QT_INSTALL_DATA]/mkspecs
mkspecs.files=$$QT_SOURCE_TREE/mkspecs/* $$QT_BUILD_TREE/mkspecs/.qt.config
mkspecs.commands = $(SYMLINK) $$QMAKESPEC $(INSTALL_ROOT)$$mkspecs.path/default
INSTALLS += mkspecs

false:macx { #mac install location
    macdocs.files = $$htmldocs.files
    macdocs.path = /Developer/Documentation/Qt
    INSTALLS += macdocs
}


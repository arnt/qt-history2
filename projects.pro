#####################################################################
# Main projectfile
#####################################################################

CONFIG += ordered
TEMPLATE = subdirs
isEmpty(QT_PROJECTS) {
#  QT_PROJECTS = qmake
   QT_PROJECTS += src tools demos examples
}
SUBDIRS += $$QT_PROJECTS

unix {
  confclean.depends += clean
  confclean.commands += $(DEL_FILE) .qmake.cache
  QMAKE_EXTRA_UNIX_TARGETS += confclean
}
CONFIG -= qt 

### installations ####

#docs
htmldocs.files = $$QT_BUILD_TREE/doc/html/*
htmldocs.path = $$docs.path/html
INSTALLS += htmldocs

#translations
translations.files = $$QT_BUILD_TREE/translations/*.qm
INSTALLS += translations


#qmake
qmake.path=$$bins.path
qmake.files=$$QT_BUILD_TREE/bin/qmake #exe itself
INSTALLS += qmake

#mkspecs
mkspecs.path=$$data.path
mkspecs.files=$$QT_SOURCE_TREE/mkspecs
mkspecs.commands = $(SYMLINK) $$QMAKESPEC $$mkspecs.path/mkspecs/default
INSTALLS += mkspecs

macx { #mac install location
    macdocs.files = $$htmldocs.files
    macdocs.path = /Developer/Documentation/Qt
    INSTALLS += macdocs
}


#####################################################################
# Main projectfile
#####################################################################

CONFIG += ordered
TEMPLATE = subdirs
isEmpty(QT_PROJECTS):QT_PROJECTS = src tools demos examples
SUBDIRS += $$QT_PROJECTS
SUBDIRS += ../research/atomic
SUBDIRS += ../research/object
SUBDIRS += ../research/eventloop
# SUBDIRS += ../research/thread
SUBDIRS += ../qtest/qtestlib

unix {
  confclean.depends += clean
  confclean.commands += $(DEL_FILE) .qmake.cache
  QMAKE_EXTRA_UNIX_TARGETS += confclean
}
CONFIG -= qt #don't need the qt.prf, but do it last





### installations ####

#docs
htmldocs.files = $$QT_BUILD_TREE/doc/html/*
htmldocs.path = $$docs.path/html
INSTALLS += htmldocs

#translations
translations.files = $$QT_BUILD_TREE/translations/*.qm
INSTALLS += translations

macx { #mac install location
    macdocs.files = $$htmldocs.files
    macdocs.path = /Developer/Documentation/Qt
    INSTALLS += macdocs
}


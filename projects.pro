#####################################################################
# Main projectfile
# ----------------
# *****************
# * MSVC.net NOTE ************************************************
# * Use 'qmake -tp vc projects.pro' to generate a solution file  *
# * containing all the projects in the subdirectories specified  *
# * below. (Note: QMAKESPEC must be set to 'win32-msvc.net')     *
# ****************************************************************
#####################################################################

CONFIG += ordered
TEMPLATE = subdirs
isEmpty(QT_PROJECTS):QT_PROJECTS = qmake src tools examples tutorials
SUBDIRS += $$QT_PROJECTS

unix {
  confclean.depends += clean
  confclean.commands += $(DEL_FILE) .qmake.cache
  QMAKE_EXTRA_UNIX_TARGETS += confclean
}
CONFIG -= qt #don't need the qt.prf, but do it last

#####################################################################
# Main projectfile
# ----------------
#
# * Use 'qmake -tp vc projects.pro' to generate a solution file 
#   containing all the projects in the subdirectories specified
#   below. (Note: QMAKESPEC must be set to 'win32-msvc.net')
#
#####################################################################

TEMPLATE = subdirs

SUBDIRS  = src/qt.pro \
	   src/qtmain.pro \
	   plugins/src \
	   tools \
	   examples \
	   tutorial \
	  
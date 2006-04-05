TEMPLATE = subdirs

SUBDIRS	*= accessible imageformats sqldrivers
# Codec plugins have been disabled for this release
# SUBDIRS *= codecs
embedded:SUBDIRS *=  gfxdrivers decorations mousedrivers
!win32:!embedded:!mac:SUBDIRS *= inputmethods

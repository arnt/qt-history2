TEMPLATE = subdirs

SUBDIRS	*= accessible codecs imageformats sqldrivers
embedded:SUBDIRS *=  gfxdrivers decorations
!win32:!embedded:!mac:SUBDIRS *= inputmethods

TEMPLATE = subdirs

SUBDIRS	*= accessible codecs imageformats sqldrivers
embedded:SUBDIRS *=  gfxdrivers decorations
x11:SUBDIRS *= inputmethods


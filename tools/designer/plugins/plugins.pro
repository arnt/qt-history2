TEMPLATE	= subdirs
PLUGIN_DIRS = wizards \
	      cppeditor \
	      dlg \
	      rc
shared:SUBDIRS *= $$PLUGIN_DIRS
dll:SUBDIRS *= $$PLUGIN_DIRS

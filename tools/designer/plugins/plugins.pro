TEMPLATE	= subdirs
PLUGIN_DIRS = wizards \
	      cppeditor \
	      dlg \
	      rc \
	      kdevdlg
shared:SUBDIRS *= $$PLUGIN_DIRS
dll:SUBDIRS *= $$PLUGIN_DIRS

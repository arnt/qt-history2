# Project ID used by some IDEs
GUID 	    = {be4c0844-619d-4b8a-bd1b-4aac1c6a5093}
TEMPLATE    = subdirs
PLUGIN_DIRS = wizards \
	      cppeditor \
	      dlg \
	      glade \
	      rc \
	      kdevdlg
shared:SUBDIRS *= $$PLUGIN_DIRS
dll:SUBDIRS *= $$PLUGIN_DIRS

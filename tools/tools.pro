# Project ID used by some IDEs
GUID 		= {a4ac224e-0e8e-48b7-96d5-a73818335a87}
TEMPLATE	= subdirs
no-png {
    message("Tools not available without PNG support")
} else {
    SUBDIRS		= assistant/lib \
		      designer \
		      assistant \
		      linguist
    unix:SUBDIRS	+= qtconfig
}

CONFIG+=ordered
QTDIR_build:REQUIRES=full-config nocrosscompiler

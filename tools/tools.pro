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

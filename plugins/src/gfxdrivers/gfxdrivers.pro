# Project ID used by some IDEs
GUID 	 = {a66a76b9-90b9-49d9-b5fb-6366e9aea0ef}
TEMPLATE = subdirs
contains(gfx-plugins, qvfb)	    :SUBDIRS += qvfb
contains(gfx-plugins, vnc)	    :SUBDIRS += vnc
contains(gfx-plugins, transformed)  :SUBDIRS += transformed
contains(gfx-plugins, mach64)	    :SUBDIRS += mach64
contains(gfx-plugins, voodoo)	    :SUBDIRS += voodoo
contains(gfx-plugins, matrox)	    :SUBDIRS += matrox
contains(gfx-plugins, shadowfb)	    :SUBDIRS += shadowfb
contains(gfx-plugins, vga16)	    :SUBDIRS += vga16

TEMPLATE = subdirs
contains(decoration-plugins, default)	:SUBDIRS += default
contains(decoration-plugins, windows)	:SUBDIRS += windows
contains(decoration-plugins, kde)	:SUBDIRS += kde
contains(decoration-plugins, beos)	:SUBDIRS += beos
contains(decoration-plugins, hydro)	:SUBDIRS += hydro

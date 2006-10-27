TEMPLATE    =	subdirs
CONFIG  += ordered
SUBDIRS	=	lib \
                theplugin \
		tst
TARGET = tst_qpluginloader

# no special install rule for subdir
INSTALLS =

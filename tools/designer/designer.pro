# Project ID used by some IDEs
GUID 	 = {da48f184-5ee0-4b66-815d-eaebf7bd1ce8}
TEMPLATE = subdirs

CONFIG  += ordered

SUBDIRS	=  uic \
	   uilib \
	   designer \
	   app

dll:SUBDIRS *=  editor plugins
shared:SUBDIRS *=  editor plugins

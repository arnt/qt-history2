TEMPLATE = subdirs

SUBDIRS += webbrowser \
           comapp \
	   multiple \
	   simple \
	   wrapper \
	   menus \
	   hierarchy

contains(QT_CONFIG, opengl):SUBDIRS += opengl

#object saftey headers are not in mingw
win32-g++:SUBDIRS -= opengl


TEMPLATE = subdirs

SUBDIRS += webbrowser \
	   qutlook \
	   multiple \
	   simple \
	   tetrax \
	   wrapper \
	   menus \
	   hierarchy

opengl:SUBDIRS += opengl

#object saftey headers are not in mingw
win32-g++:SUBDIRS -= opengl


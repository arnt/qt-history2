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

#ohject saftey headers are not in mingw
win32-g++:SUBDIRS -= opengl


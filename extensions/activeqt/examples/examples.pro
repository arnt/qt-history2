GUID 	 = {a4301b32-0643-4759-8064-5e74da7a8df9}
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


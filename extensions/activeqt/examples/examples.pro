TEMPLATE = subdirs
SUBDIRS += testcon \
	  webbrowser \
	  qutlook \
	  dumpdoc \
	  multiple \
	  simple \
	  tetrax \
	  wrapper \
	  menus \
	  hierarchy
 
opengl:SUBDIRS += opengl

#mingw dose not suport controls yet
win32-g++:SUBDIRS -= multiple \
	  simple \
	  tetrax \
	  wrapper \
	  menus \
	  hierarchy \
	  opengl



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

win32-g++:SUBDIRS -= simple \
			tetrax \
			menus \
			opengl



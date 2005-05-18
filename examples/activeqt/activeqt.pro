TEMPLATE      = subdirs
SUBDIRS      += comapp \
                hierarchy \
                menus \
                multiple \
                simple \
                webbrowser \
                wrapper

contains(QT_CONFIG, opengl):SUBDIRS += opengl

# For now only the contain examples with mingw, for the others you need
# an IDL compiler
win32-g++:SUBDIRS = webbrowser

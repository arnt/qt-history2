# Compiling
SYSCONF_CC		= #$ Expand('TMAKE_CC');

# Compiling with support libraries
SYSCONF_CFLAGS_X11	= #$ ExpandGlue('TMAKE_INCDIR_X11', '-I', ' -I', '');
SYSCONF_CFLAGS_QT	= #$ ExpandGlue('TMAKE_INCDIR_QT', '-I', ' -I', '');
SYSCONF_CFLAGS_OPENGL	= #$ ExpandGlue('TMAKE_INCDIR_OPENGL', '-I', ' -I', '');

# Linking with support libraries
# X11
SYSCONF_LFLAGS_X11	= #$ ExpandGlue('TMAKE_LIBDIR_X11', '-L', ' -L', '');
SYSCONF_LIBS_X11	= #$ Expand('TMAKE_LIBS_X11');
# Qt, Qt+OpenGL
SYSCONF_LFLAGS_QT	= #$ ExpandGlue('TMAKE_LIBDIR_QT', '-L', ' -L', '');
SYSCONF_LIBS_QT		= #$ Expand('TMAKE_LIBS_QT');
SYSCONF_LIBS_QT_OPENGL	= #$ Expand('TMAKE_LIBS_QT_OPENGL');
# OpenGL
SYSCONF_LFLAGS_OPENGL	= #$ ExpandGlue('TMAKE_LIBDIR_OPENGL', '-L', ' -L', '');
SYSCONF_LIBS_OPENGL	= #$ Expand('TMAKE_LIBS_OPENGL');

# Linking applications
SYSCONF_LINK		= #$ Expand('TMAKE_LINK');
SYSCONF_LFLAGS		= #$ Expand('TMAKE_LFLAGS');
SYSCONF_LIBS		= #$ Expand('TMAKE_LIBS');

# Meta-object compiler
SYSCONF_MOC		= #$ Expand('TMAKE_MOC');

SYSCONF_LINK_LIB	= $(SYSCONF_LINK_LIB_STATIC)
SYSCONF_LINK_TARGET	= $(SYSCONF_LINK_TARGET_STATIC)
# Compiling library source
SYSCONF_CXXFLAGS_LIB	= 
SYSCONF_CFLAGS_LIB	= 
# Compiling shared-object source
SYSCONF_CXXFLAGS_SHOBJ	= #$ Expand('TMAKE_CXXFLAGS_SHLIB');
SYSCONF_CFLAGS_SHOBJ	= #$ Expand('TMAKE_CFLAGS_SHLIB');
# Linking Qt
SYSCONF_LIBS_QTLIB	= $(SYSCONF_CXXFLAGS_X11) $(QT_LIBS_MT) $(QT_LIBS_OPT)
# Linking Qt applications
SYSCONF_LIBS_QTAPP	= $(SYSCONF_CXXFLAGS_X11) $(QT_LIBS_MT) $(QT_LIBS_OPT)

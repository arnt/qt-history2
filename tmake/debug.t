# Compiling application source
SYSCONF_CXXFLAGS	= #$ Expand('TMAKE_CXXFLAGS'); Expand('TMAKE_CXXFLAGS_DEBUG');
SYSCONF_CFLAGS		= #$ Expand('TMAKE_CFLAGS'); Expand('TMAKE_CFLAGS_DEBUG');
# Compiling library source
SYSCONF_CXXFLAGS_LIB	= #$ Expand('TMAKE_CXXFLAGS_SHLIB');
SYSCONF_CFLAGS_LIB	= #$ Expand('TMAKE_CFLAGS_SHLIB');
# Compiling shared-object source
SYSCONF_CXXFLAGS_SHOBJ	= #$ Expand('TMAKE_CXXFLAGS_SHLIB');
SYSCONF_CFLAGS_SHOBJ	= #$ Expand('TMAKE_CFLAGS_SHLIB');

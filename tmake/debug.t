# Compiling application source
SYSCONF_CXXFLAGS	= #$ Expand('TMAKE_CXXFLAGS'); Expand('TMAKE_CXXFLAGS_DEBUG');
# Compiling library source
SYSCONF_CXXFLAGS_LIB	= #$ Expand('TMAKE_CXXFLAGS_DEBUG');  Expand('TMAKE_CXXFLAGS_SHLIB');
# Compiling shared-object source
SYSCONF_CXXFLAGS_SHOBJ	= #$ Expand('TMAKE_CXXFLAGS_DEBUG'); Expand('TMAKE_CXXFLAGS_SHLIB');

# Compiling application source
SYSCONF_CFLAGS		= #$ Expand('TMAKE_CFLAGS'); Expand('TMAKE_CFLAGS_RELEASE');
# Compiling library source
SYSCONF_CFLAGS_LIB	= #$ Expand('TMAKE_CFLAGS_RELEASE');  Expand('TMAKE_CFLAGS_SHLIB');
# Compiling shared-object source
SYSCONF_CFLAGS_SHOBJ	= #$ Expand('TMAKE_CFLAGS_RELEASE'); Expand('TMAKE_CFLAGS_SHLIB');

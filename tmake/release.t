# Compiling application source
SYSCONF_CFLAGS		= #$ Expand('TMAKE_CFLAGS'); Expand('TMAKE_CFLAGS_RELEASE');
# Compiling shared-object source
SYSCONF_CFLAGS_SHOBJ	= #$ Expand('TMAKE_CFLAGS_RELEASE'); Expand('TMAKE_CFLAGS_SHLIB');

# Compiling application source
SYSCONF_CFLAGS		= #$ Expand('TMAKE_CFLAGS'); Expand('TMAKE_CFLAGS_DEBUG');
# Compiling shared-object source
SYSCONF_CFLAGS_SHOBJ	= #$ Expand('TMAKE_CFLAGS_DEBUG'); Expand('TMAKE_CFLAGS_SHLIB');

#$ IncludeTemplate("common.t");
#$ IncludeTemplate("debug.t");
#$ IncludeTemplate("sharedlib.t");

# Compiling library source
SYSCONF_CFLAGS_LIB	= #$ Expand('TMAKE_CFLAGS_DEBUG');  Expand('TMAKE_CFLAGS_SHLIB');

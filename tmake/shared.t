#$ IncludeTemplate("common.t");
#$ IncludeTemplate("release.t");
#$ IncludeTemplate("sharedlib.t");

# Compiling library source
SYSCONF_CFLAGS_LIB	= #$ Expand('TMAKE_CFLAGS_RELEASE');  Expand('TMAKE_CFLAGS_SHLIB');

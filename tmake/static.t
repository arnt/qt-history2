#$ IncludeTemplate("common.t");
#$ IncludeTemplate("release.t");
#$ IncludeTemplate("staticlib.t");

# Compiling library source
SYSCONF_CFLAGS_LIB	= #$ Expand('TMAKE_CFLAGS_RELEASE');

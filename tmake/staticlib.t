# Linking libraries
#   - Build the $(TARGET) library, eg. lib$(TARGET).a
#   - Place target in $(DESTDIR) - which has a trailing /
#   - May need to incorporate $(VER_MAJ) and $(VER_MIN) for shared libraries.
#
SYSCONF_LINK_TARGET	= #$ $text .= 'lib$(TARGET).a' . "\n";
SYSCONF_LINK_LIB	= #${
        if ( $project{"TMAKE_AR_CMD"} ) {
            $project{"TMAKE_AR_CMD"} =~ s/TARGETA/TARGET/g;
        } else {
            $project{"TMAKE_AR_CMD"} =
                '$(AR) $(TARGET) $(OBJECTS) $(OBJMOC)';
        }
	$text .= 'rm -f $(DESTDIR)$(SYSCONF_LINK_TARGET); ';
	if ( $project{"TMAKE_AR_CMD"} ) {
	    $text .= " \\\n\t\t\t\t";
	    Expand("TMAKE_AR_CMD");
	}
	if ( $project{"TMAKE_RANLIB"} ) {
	    $text .= " \\\n\t\t\t\t";
	    ExpandGlue("TMAKE_RANLIB","",""," \$(TARGET)");
	}
#$}

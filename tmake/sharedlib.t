# Linking libraries
#   - Build the $(TARGET) library, eg. lib$(TARGET).a
#   - Place target in $(DESTDIR) - which has a trailing /
#   - May need to incorporate $(VER_MAJ) and $(VER_MIN) for shared libraries.
#
SYSCONF_LINK_SHLIB	= #$ Expand('TMAKE_LINK_SHLIB');
SYSCONF_LINK_TARGET	= #${
    if ( Project('TMAKE_HPUX_SHLIB') ) {
	$text .= 'lib$(TARGET).sl' . "\n";
    } else {
	$text .= 'lib$(TARGET).so.$(VER_MAJ).$(VER_MIN)' . "\n";
    }
#$}
SYSCONF_LINK_LIB	= #${
    if ( Project('TMAKE_HPUX_SHLIB') ) {
	$text .= ' $(SYSCONF_LINK_SHLIB) '
		       . Project('TMAKE_LFLAGS_SHLIB') . ' '
		       . ( Project('TMAKE_LFLAGS_SONAME')
			     ? Project('TMAKE_LFLAGS_SONAME') . 'lib$(TARGET).sl'
			     : '' )
		       . ' $(LFLAGS) -o $(SYSCONF_LINK_TARGET) $(OBJECTS) '
		       . ' $(OBJMOC) $(LIBS);'
		 . ' mv $(SYSCONF_LINK_TARGET) $(DESTDIR);'
		 . ' cd $(DESTDIR);'
		 . ' rm -f lib$(TARGET).sl';
    } else {
	if ( Project('TMAKE_LINK_SHLIB_CMD') ) {
	    $text .= ' $(SYSCONF_LINK_SHLIB)'
			. ' $(LFLAGS) -o $(SYSCONF_LINK_TARGET)'
			. ' `lorder /usr/lib/c++rt0.o $(OBJECTS) $(OBJMOC)'
			    . ' | tsort` $(LIBS); ';
	} else {
	    $text .= ' $(SYSCONF_LINK_SHLIB) '
			. Project('TMAKE_LFLAGS_SHLIB') . ' '
			. ( Project('TMAKE_LFLAGS_SONAME')
			     ? Project('TMAKE_LFLAGS_SONAME') . 'lib$(TARGET).so.$(VER_MAJ)'
			     : '' ) . " \\\n\t\t\t\t"
			. '     $(LFLAGS) -o $(SYSCONF_LINK_TARGET)' . " \\\n\t\t\t\t"
			. '     $(OBJECTS) $(OBJMOC) $(LIBS);';
	}
	$text .= " \\\n\t\t\t\t";
	$text .= ' mv $(SYSCONF_LINK_TARGET) $(DESTDIR);' . " \\\n\t\t\t\t"
		. ' cd $(DESTDIR);' . " \\\n\t\t\t\t"
		. ' rm -f lib$(TARGET).so'
		    . ' lib$(TARGET).so.$(VER_MAJ);' . " \\\n\t\t\t\t"
		. ' ln -s $(SYSCONF_LINK_TARGET) lib$(TARGET).so;' . " \\\n\t\t\t\t"
		. ' ln -s $(SYSCONF_LINK_TARGET) lib$(TARGET).so.$(VER_MAJ)';
    }
#$}

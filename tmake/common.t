# Compiling
SYSCONF_CXX		= #$ Expand('TMAKE_CXX');
SYSCONF_CC		= #$ Expand('TMAKE_CC');

# Compiling with support libraries
SYSCONF_CXXFLAGS_X11	= #$ ExpandGlue('TMAKE_INCDIR_X11', '-I', ' -I', '');
SYSCONF_CXXFLAGS_QT	= #$ ExpandGlue('TMAKE_INCDIR_QT', '-I', ' -I', '');
SYSCONF_CXXFLAGS_OPENGL	= #$ ExpandGlue('TMAKE_INCDIR_OPENGL', '-I', ' -I', '');

# Compiling YACC output
SYSCONF_CXXFLAGS_YACC     = #$ Expand('TMAKE_CXXFLAGS_YACC');

# Linking with support libraries
# X11
SYSCONF_LFLAGS_X11	= #$ ExpandGlue('TMAKE_LIBDIR_X11', '-L', ' -L', '');
SYSCONF_LIBS_X11	= #$ Expand('TMAKE_LIBS_X11');
# Qt, Qt+OpenGL
SYSCONF_LFLAGS_QT	= #$ ExpandGlue('TMAKE_LIBDIR_QT', '-L', ' -L', '');
SYSCONF_LIBS_QT		= #$ Expand('TMAKE_LIBS_QT');
SYSCONF_LIBS_QT_OPENGL	= #$ Expand('TMAKE_LIBS_QT_OPENGL');
# OpenGL
SYSCONF_LFLAGS_OPENGL	= #$ ExpandGlue('TMAKE_LIBDIR_OPENGL', '-L', ' -L', '');
SYSCONF_LIBS_OPENGL	= #$ Expand('TMAKE_LIBS_OPENGL');
# Yacc
SYSCONF_LIBS_YACC	= #$ Expand('TMAKE_LIBS_YACC');

# Linking applications
SYSCONF_LINK		= #$ Expand('TMAKE_LINK');
SYSCONF_LFLAGS		= #$ Expand('TMAKE_LFLAGS');
SYSCONF_LIBS		= #$ Expand('TMAKE_LIBS');

# Link flags for shared objects
SYSCONF_LFLAGS_SHOBJ	= #$ Expand('TMAKE_LFLAGS_SHLIB');

# Meta-object compiler
SYSCONF_MOC		= $(QTDIR)/bin/moc

# Linking shared libraries
#   - Build the $(TARGET) library, eg. lib$(TARGET).so.2.0.1
#   - Place target in $(DESTDIR) - which has a trailing /
#   - Usually needs to incorporate $(VER_MAJ), $(VER_MIN) and $(VER_PATCH)
#
SYSCONF_LINK_SHLIB	= #$ Expand('TMAKE_LINK_SHLIB');
SYSCONF_LINK_TARGET_SHARED	= #${
    if ( Project('TMAKE_HPUX_SHLIB') ) {
	$text .= 'lib$(TARGET).$(VER_MAJ)';
    } else {
	$text .= 'lib$(TARGET).so.$(VER_MAJ).$(VER_MIN).$(VER_PATCH)';
    }
#$}
SYSCONF_LINK_LIB_SHARED	= #${
    if ( Project('TMAKE_HPUX_SHLIB') ) {
	$text .= ' $(SYSCONF_LINK_SHLIB) '
		       . Project('TMAKE_LFLAGS_SHLIB') . ' '
		       . ( Project('TMAKE_LFLAGS_SONAME')
			     ? Project('TMAKE_LFLAGS_SONAME') . '$(SYSCONF_LINK_TARGET_SHARED)'
			     : '' ) . " \\\n\t\t\t\t"
		       . ' $(LFLAGS) -o $(SYSCONF_LINK_TARGET_SHARED)' . " \\\n\t\t\t\t"
		       . ' $(OBJECTS) $(OBJMOC) $(LIBS) &&' . " \\\n\t\t\t\t"
		       . ' chmod 555 $(SYSCONF_LINK_TARGET_SHARED) &&';

	$text .= " \\\n\t\t\t\t";
	$text .= ' mv $(SYSCONF_LINK_TARGET_SHARED) $(DESTDIR);' . " \\\n\t\t\t\t"
		    . ' cd $(DESTDIR) &&' . " \\\n\t\t\t\t"
		    . ' rm -f lib$(TARGET).sl;' . " \\\n\t\t\t\t"
		    . ' ln -s $(SYSCONF_LINK_TARGET_SHARED) lib$(TARGET).sl';
    } else {
	if ( Project('TMAKE_LINK_SHLIB_CMD') ) {
	    $text .= Project('TMAKE_LINK_SHLIB_CMD');
	} else {
	    $text .= ' $(SYSCONF_LINK_SHLIB) '
			. Project('TMAKE_LFLAGS_SHLIB') . ' '
			. ( Project('TMAKE_LFLAGS_SONAME')
			     ? Project('TMAKE_LFLAGS_SONAME') . 'lib$(TARGET).so.$(VER_MAJ)'
			     : '' ) . " \\\n\t\t\t\t"
			. '     $(LFLAGS) -o $(SYSCONF_LINK_TARGET_SHARED)' . " \\\n\t\t\t\t"
			. '     $(OBJECTS) $(OBJMOC) $(LIBS) &&';
	    $text .= " \\\n\t\t\t\t";
	    $text .= ' mv $(SYSCONF_LINK_TARGET_SHARED) $(DESTDIR);' . " \\\n\t\t\t\t"
		    . ' cd $(DESTDIR) &&' . " \\\n\t\t\t\t"
		    . ' rm -f lib$(TARGET).so lib$(TARGET).so.$(VER_MAJ)'
			. ' lib$(TARGET).so.$(VER_MAJ).$(VER_MIN);' . " \\\n\t\t\t\t"
		    . ' ln -s $(SYSCONF_LINK_TARGET_SHARED) lib$(TARGET).so;' . " \\\n\t\t\t\t"
		    . ' ln -s $(SYSCONF_LINK_TARGET_SHARED) lib$(TARGET).so.$(VER_MAJ);' . " \\\n\t\t\t\t"
		    . ' ln -s $(SYSCONF_LINK_TARGET_SHARED) lib$(TARGET).so.$(VER_MAJ).$(VER_MIN)';
	}
    }
#$}

# Linking static libraries
#   - Build the $(TARGET) library, eg. lib$(TARGET).a
#   - Place target in $(DESTDIR) - which has a trailing /
#
SYSCONF_AR		= #$ Expand('TMAKE_AR');
SYSCONF_LINK_TARGET_STATIC = lib$(TARGET).a
SYSCONF_LINK_LIB_STATIC	= #${
        if ( $project{"TMAKE_AR_CMD"} ) {
            $project{"TMAKE_AR_CMD"} =~ s/\$\(TARGETA\)/\$(DESTDIR)$targ/g;
        } else {
            $project{"TMAKE_AR_CMD"} =
                '$(SYSCONF_AR) $(DESTDIR)$(SYSCONF_LINK_TARGET_STATIC) $(OBJECTS) $(OBJMOC) ';
        }
	$text .= 'rm -f $(DESTDIR)$(SYSCONF_LINK_TARGET_STATIC) ';
	if ( $project{"TMAKE_AR_CMD"} ) {
	    $text .= "; \\\n\t\t\t\t";
	    Expand("TMAKE_AR_CMD");
	}
	if ( $project{"TMAKE_RANLIB"} ) {
	    $text .= "; \\\n\t\t\t\t";
	    ExpandGlue("TMAKE_RANLIB","","",' $(DESTDIR)$(SYSCONF_LINK_TARGET_STATIC)');
	}
#$}

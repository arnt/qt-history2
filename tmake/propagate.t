#############################################################################
#!
#! This is a tmake template for building UNIX libraries.
#!
#! Actually, it will be trivial to make app vs. lib a config option.
#!
#${
    $project{"TMAKE_CXXFLAGS"} = "";
    $project{"TMAKE_CFLAGS"} = "";
    $project{"TMAKE_LFLAGS"} = "";
    $project{"TMAKE_LIBS"} = "";

    Project('TMAKE_LIBS += $$LIBS'); # Misc. project-specific extras
    if ( Project('TEMPLATE') eq "qt.t" ) {
	# Qt/Embedded hackery.
	Project('TMAKE_LIBS += -L'.$project{"OBJECTS_DIR"});
	Project('TMAKE_LIBS += -freetype2');
    }

    if ( Config("qt") || Config("opengl") ) {
	Project('CONFIG *= x11lib');
	if ( Config("opengl") ) {
	    Project('CONFIG *= x11inc');
	}
    }
    if ( Config("x11") ) {
	Project('CONFIG *= x11lib');
	Project('CONFIG *= x11inc');
    }
    if ( Config("qt") ) {
	$moc_aware = 1;
	Project('TMAKE_CXXFLAGS *= $(SYSCONF_CXXFLAGS_QT)');
	if ( Config("opengl") ) {
	    Project('TMAKE_LFLAGS *= $(SYSCONF_LFLAGS_QT)');
	    if ( Project("TARGET") ne "qgl" ) {
		Project('TMAKE_LIBS *= $(SYSCONF_LIBS_QT_OPENGL)');
	    }
	}
	if ( Project("TARGET") eq "qt" ) {
	    $project{"PNG_OBJECTS"} = &Objects($project{"PNG_SOURCES"});
	    $project{"ZLIB_OBJECTS"} = &Objects($project{"ZLIB_SOURCES"});
	} else {
	    Project('TMAKE_LFLAGS *= $(SYSCONF_LFLAGS_QT)');
	    Project('TMAKE_LIBS *= $(SYSCONF_LIBS_QT)');
	}
    } elsif ( Config("qtinc") ) {
	Project('TMAKE_CXXFLAGS *= $(SYSCONF_CXXFLAGS_QT)');
    }
    if ( Config("opengl") ) {
	Project('TMAKE_CXXFLAGS *= $(SYSCONF_CXXFLAGS_OPENGL)');
	Project('TMAKE_LFLAGS *= $(SYSCONF_LFLAGS_OPENGL)');
	Project('TMAKE_LIBS *= $(SYSCONF_LIBS_OPENGL)');
    }
    if ( Config("x11inc") ) {
	Project('TMAKE_CXXFLAGS *= $(SYSCONF_CXXFLAGS_X11)');
    }
    if ( Config("x11lib") ) {
	Project('TMAKE_LFLAGS *= $(SYSCONF_LFLAGS_X11)');
	Project('TMAKE_LIBS *= $(SYSCONF_LIBS_X11)');
    }

    ##### These may still need replacing
    if ( !Project("TMAKE_RUN_CC") ) {
	Project('TMAKE_RUN_CC = $(CC) -c $(CFLAGS) $(INCPATH) -o $obj $src');
    }
    if ( !Project("TMAKE_RUN_CC_IMP") ) {
	Project('TMAKE_RUN_CC_IMP = $(CC) -c $(CFLAGS) $(INCPATH) -o $@ $<');
    }
    if ( !Project("TMAKE_RUN_CXX") ) {
	Project('TMAKE_RUN_CXX = $(CXX) -c $(CXXFLAGS) $(INCPATH) -o $obj $src');
    }
    if ( !Project("TMAKE_RUN_CXX_IMP") ) {
	Project('TMAKE_RUN_CXX_IMP = $(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<');
    }

    Project('TMAKE_FILETAGS = HEADERS SOURCES TARGET DESTDIR $$FILETAGS');
    Project('DESTDIR = ./') unless Project('DESTDIR');

    StdInit();

    $project{"DESTDIR"} = FixPath($project{"DESTDIR"});
    $project{"VERSION"} || ($project{"VERSION"} = "1.0.0");
    ($project{"VER_MAJ"},
     $project{"VER_MIN"},
     $project{"VER_PATCH"}) = $project{"VERSION"} =~ /(\d+)\.(\d+)(?:\.(\d+))?/;
    $project{"VER_PATCH"} = 0 if !$project{"VER_PATCH"};
    if ( Config("dll") ) {
	Project('TMAKE_CXXFLAGS *= $(SYSCONF_CXXFLAGS_SHOBJ)' );
	Project('TMAKE_CFLAGS *= $(SYSCONF_CFLAGS_SHOBJ)' );
	Project('TMAKE_LFLAGS *= $(SYSCONF_LFLAGS_SHOBJ)');
    }
    if ( Config("yacc") ) {
	Project('TMAKE_CXXFLAGS *= $(SYSCONF_CXXFLAGS_YACC)' );
	Project('TMAKE_LIBS *= $(SYSCONF_LIBS_YACC)' );
    }
    Project('TMAKE_CXXFLAGS *= $(SYSCONF_CXXFLAGS)' );
    Project('TMAKE_CFLAGS *= $(SYSCONF_CFLAGS)' );
    Project('TMAKE_LFLAGS *= $(SYSCONF_LFLAGS)' );
    if ( Project('TEMPLATE') eq "lib" || Project('TEMPLATE') eq "qt.t" ) {
	Project('TMAKE_CXXFLAGS *= $(SYSCONF_CXXFLAGS_LIB)' );
	Project('TMAKE_CFLAGS *= $(SYSCONF_CFLAGS_LIB)' );
    } else {
	Project('TMAKE_LIBS *= $(SYSCONF_LIBS)' );
    }
#$}

####### Compiler, tools and options

CXX	=	$(SYSCONF_CXX)
CXXFLAGS=	#$ Expand("TMAKE_CXXFLAGS"); ExpandGlue("DEFINES","-D"," -D",""); (Project("TARGET") eq "qt") && ($text = $text . ' $(QT_CXXFLAGS_OPT)');
CC	=	$(SYSCONF_CC)
CFLAGS	=	#$ Expand("TMAKE_CFLAGS"); ExpandGlue("DEFINES","-D"," -D",""); (Project("TARGET") eq "qt") && ($text = $text . ' $(QT_CFLAGS_OPT)');
INCPATH =	#$ ExpandGlue("INCPATH","-I"," -I","");
LFLAGS	=	#$ Expand("TMAKE_LFLAGS");
LIBS	=	#$ (Project("TARGET") eq "qt") && ($text = $text . ' $(QT_LIBS_OPT)'); Expand("TMAKE_LIBS");
MOC	=	$(SYSCONF_MOC)

####### Target

DESTDIR = #$ Expand("DESTDIR");
VER_MAJ = #$ Expand("VER_MAJ");
VER_MIN = #$ Expand("VER_MIN");
VER_PATCH = #$ Expand("VER_PATCH");
TARGET	= #$ Expand("TARGET");
TARGET1 = lib$(TARGET).so.$(VER_MAJ)

####### Files

HEADERS =	#$ ExpandList("HEADERS");
SOURCES =	#$ ExpandList("SOURCES");
OBJECTS =	#$ ExpandList("OBJECTS"); (Project("TARGET") eq "qt") && ($text = $text . ' $(QT_PNG_OBJ) $(QT_ZLIB_OBJ)');
SRCMOC	=	#$ ExpandList("SRCMOC");
OBJMOC	=	#$ ExpandList("OBJMOC");
#$ (Project("TARGET") eq "qt") || DisableOutput();
PNG_OBJECTS  = #$ ExpandList("PNG_OBJECTS");
ZLIB_OBJECTS = #$ ExpandList("ZLIB_OBJECTS");
#$ (Project("TARGET") eq "qt") || EnableOutput();

####### Implicit rules

.SUFFIXES: .cpp .cxx .cc .C .c

.cpp.o:
	#$ Expand("TMAKE_RUN_CXX_IMP");

.cxx.o:
	#$ Expand("TMAKE_RUN_CXX_IMP");

.cc.o:
	#$ Expand("TMAKE_RUN_CXX_IMP");

.C.o:
	#$ Expand("TMAKE_RUN_CXX_IMP");

.c.o:
	#$ Expand("TMAKE_RUN_CC_IMP");

####### Build rules

#${
	if ( Project('TEMPLATE') eq "lib" || Project('TEMPLATE') eq "qt.t" ) {
	    if ( Config('staticlib') ) {
		$targ = '$(SYSCONF_LINK_TARGET_STATIC)';
	    } else {
		$targ='$(SYSCONF_LINK_TARGET)';
	    }
	} else {
	    if ( Config('dll') ) {
		if ($is_unix) {
		    $targ = '$(TARGET)' . ".so";
		} else {
		    $targ = '$(TARGET)' . ".dll";
		    $targ = "np" . $targ if Config('np');
		}
	    } else {
		$targ = '$(TARGET)'
	    }
	}
	$text .= 'all: ';
       	ExpandGlue("ALL_DEPS",""," "," ");
	$text .= '$(DESTDIR)' . $targ . "\n\n";
	$text .= '$(DESTDIR)' . $targ . ': $(OBJECTS) $(OBJMOC) ';
	if ( Project('TEMPLATE') eq "qt.t" ) {
	    # Qt/Embedded hackery.
	    $text .= '$(OBJECTS_DIR)/libfreetype.a ';
	}
	Expand("TARGETDEPS");
	$text .= "\n\t";
	if ( Project('TEMPLATE') eq "lib" || Project('TEMPLATE') eq "qt.t" ) {
	    if ( Config('staticlib') ) {
		$text .= '$(SYSCONF_LINK_LIB_STATIC)';
	    } else {
		$text .= '$(SYSCONF_LINK_LIB)';
	    }
	} else {
	    $text .= '$(SYSCONF_LINK) $(LFLAGS) ';
	    $text .= '-o '.$targ.' $(OBJECTS) $(OBJMOC) $(LIBS)';
	}
#$}

#$ Substitute('$$OBJECTS_DIR/libfreetype.a:');
#$ Substitute('	cd 3rdparty/freetype2; make CONFIG_MK=config.mk OBJ_DIR=../../$$OBJECTS_DIR ../../$$OBJECTS_DIR/libfreetype.a');

moc: $(SRCMOC)

#$ TmakeSelf();

clean:
	-rm -f $(OBJECTS) $(OBJMOC) $(SRCMOC)
	#$ ExpandGlue("TMAKE_CLEAN","-rm -f "," ","");
	-rm -f *~ core
	#$ ExpandGlue("CLEAN_FILES","-rm -f "," ","");

# For Qt/Embedded only
tmp/qt.cpp: ../include/qt.h $(HEADERS)
	-mkdir tmp
	echo "#include <qt.h>" >tmp/qt.cpp
	$(CXX) -E -DQT_MOC_CPP $(CXXFLAGS) $(INCPATH) -o tmp/moc_qt.h tmp/qt.cpp
	$(MOC) -o tmp/qt.cpp-tmp tmp/moc_qt.h
	sed -e 's/"moc_qt.h"/"qt.h"/' <tmp/qt.cpp-tmp >tmp/qt.cpp

####### Compile

#$ BuildObj(Project("OBJECTS"),Project("SOURCES"));
#$ BuildMocObj(Project("OBJMOC"),Project("SRCMOC"));
#$ BuildMocSrc(Project("HEADERS"));
#$ BuildMocSrc(Project("SOURCES"));
#$ Project("PNG_SOURCES") && BuildObj(Project("PNG_OBJECTS"),Project("PNG_SOURCES"));
#$ Project("ZLIB_SOURCES") && BuildObj(Project("ZLIB_OBJECTS"),Project("ZLIB_SOURCES"));

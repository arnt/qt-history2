#############################################################################
#!
#! This is a tmake template for building UNIX libraries.
#!
#! Actually, it will be trivial to make app vs. lib a config option.
#!
#${
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
	Project('TMAKE_CFLAGS *= $(SYSCONF_CFLAGS_QT)');
	if ( Config("opengl") ) {
	    Project('TMAKE_LFLAGS *= $(SYSCONF_LFLAGS_QT)');
	    Project('TMAKE_LIBS *= $(SYSCONF_LIBS_QT_OPENGL)');
	}
	if ( Project("TARGET") ne "qt" ) {
	    Project('TMAKE_LFLAGS *= $(SYSCONF_LFLAGS_QT)');
	    Project('TMAKE_LIBS *= $(SYSCONF_LIBS_QT)');
	}
    } elsif ( Config("qtinc") ) {
	# eg. moc only needs the includes
	Project('TMAKE_CFLAGS *= $(SYSCONF_CFLAGS_QT)');
    }
    if ( Config("opengl") ) {
	Project('TMAKE_CFLAGS *= $(SYSCONF_CFLAGS_OPENGL)');
	Project('TMAKE_LFLAGS *= $(SYSCONF_LFLAGS_OPENGL)');
	Project('TMAKE_LIBS *= $(SYSCONF_LIBS_OPENGL)');
    }
    if ( Config("x11inc") ) {
	Project('TMAKE_CFLAGS *= $(SYSCONF_CFLAGS_X11)');
    }
    if ( Config("x11lib") ) {
	Project('TMAKE_LFLAGS *= $(SYSCONF_LFLAGS_X11)');
	Project('TMAKE_LIBS *= $(SYSCONF_LIBS_X11)');
    }
    Project('TMAKE_LIBS += $$LIBS'); # Misc. project-specific extras

    ##### These may still need replacing
    if ( !Project('TMAKE_COMPILE') ) {
	Project('TMAKE_COMPILE = $(CC) -c $(CFLAGS) -o $obj $src');
    }
    if ( !Project('TMAKE_COMPILE_IMP') ) {
	Project('TMAKE_COMPILE_IMP = $(CC) -c $(CFLAGS) -o $@ $<');
    }

    Project('TMAKE_FILETAGS = HEADERS SOURCES TARGET DESTDIR $$FILETAGS');
    Project('DESTDIR = ./') unless Project('DESTDIR');

    StdInit();

    $project{"DESTDIR"} = FixPath($project{"DESTDIR"});
    $project{"VERSION"} || ($project{"VERSION"} = "1.0");
    $project{"VER_MAJ"} = $project{"VERSION"};
    $project{"VER_MAJ"} =~ s/\.\d+$//;
    $project{"VER_MIN"} = $project{"VERSION"};
    $project{"VER_MIN"} =~ s/^\d+\.//;
    if ( Config("dll") ) {
	Project('TMAKE_CFLAGS *= $(SYSCONF_CFLAGS_SHOBJ)' );
    }
    if ( Config("yacc") ) {
	Project('TMAKE_CFLAGS *= $(SYSCONF_CFLAGS_YACC)' );
    }
    if ( Project('TEMPLATE') eq "lib" ) {
	Project('TMAKE_CFLAGS *= $(SYSCONF_CFLAGS_LIB)' );
    } else {
	Project('TMAKE_CFLAGS *= $(SYSCONF_CFLAGS)' );
	Project('TMAKE_LFLAGS *= $(SYSCONF_LFLAGS)' );
	Project('TMAKE_LIBS *= $(SYSCONF_LIBS)' );
    }
#$}
#!
# Makefile for building #$ Expand("TARGET")
# Generated by tmake at #$ Now();
#     Project: #$ Expand("PROJECT");
#    Template: #$ Expand("TEMPLATE");
#############################################################################

####### System-dependendant options

####### BEGIN
build_error:
	@echo "This section should have been replaced by a config file."
####### END

####### Compiler, tools and options

CC	=	$(SYSCONF_CC)
CFLAGS	=	#$ Expand("TMAKE_CFLAGS"); ExpandGlue("DEFINES","-D"," -D","");
LFLAGS	=	#$ Expand("TMAKE_LFLAGS");
LIBS	=	#$ Expand("TMAKE_LIBS");
MOC	=	$(SYSCONF_MOC)

####### Target

TARGET	= #$ Expand("TARGET");
DESTDIR = #$ Expand("DESTDIR");
VER_MAJ = #$ Expand("VER_MAJ");
VER_MIN = #$ Expand("VER_MIN");

####### Files

HEADERS =	#$ ExpandList("HEADERS");
SOURCES =	#$ ExpandList("SOURCES");
OBJECTS =	#$ ExpandList("OBJECTS");
SRCMOC	=	#$ ExpandList("SRCMOC");
OBJMOC	=	#$ ExpandList("OBJMOC");

####### Implicit rules

.SUFFIXES: .cpp .cxx .cc .C .c

.cpp.o:
	#$ Expand("TMAKE_COMPILE_IMP");

.cxx.o:
	#$ Expand("TMAKE_COMPILE_IMP");

.cc.o:
	#$ Expand("TMAKE_COMPILE_IMP");

.C.o:
	#$ Expand("TMAKE_COMPILE_IMP");

.c.o:
	#$ Expand("TMAKE_COMPILE_IMP");

####### Build rules

all: #$ ExpandGlue("ALL_DEPS",""," "," "); $text .= '$(DESTDIR)$(SYSCONF_LINK_TARGET)';

$(DESTDIR)$(SYSCONF_LINK_TARGET): $(OBJECTS) $(OBJMOC) #$ Expand("TARGETDEPS");
	#${
	    if ( Project('TEMPLATE') eq "lib" ) {
		$text .= '$(SYSCONF_LINK_LIB)';
	    } else {
		$text .= '$(SYSCONF_LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(OBJMOC) $(LIBS)';
	    }
#$}

moc: $(SRCMOC)

#$ TmakeSelf();

clean:
	-rm -f $(OBJECTS) $(OBJMOC) $(SRCMOC)
	#$ ExpandGlue("TMAKE_CLEAN","-rm -f "," ","");
	-rm -f *~ core
	#$ ExpandGlue("CLEAN_FILES","-rm -f "," ","");

####### Compile

#$ BuildObj(Project("OBJECTS"),Project("SOURCES"));
#$ BuildMocObj(Project("OBJMOC"),Project("SRCMOC"));
#$ BuildMocSrc(Project("HEADERS"));
#$ BuildMocSrc(Project("SOURCES"));

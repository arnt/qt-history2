@rem = '--*-PERL-*--';
@rem = '
@echo off
rem setlocal
set ARGS=
:loop
if .%1==. goto endloop
set ARGS=%ARGS% %1
shift
goto loop
:endloop
rem ***** This assumes PERL is in the PATH *****
perl.exe -S makeqtmake.bat %ARGS%
goto endofperl
@rem ';
#!/usr/bin/perl
############################################################################
# $Id: //depot/qt/main/bin/makeqtmake.bat#1 $
#
# Makes Qt makefiles and a project file - internal Troll Tech tool.
#
# Usage:
#    makeqtmake [-lib] [-cvs] projectname
#
# Copyright (C) 1998 by Troll Tech AS.  All rights reserved.
#
############################################################################

$libtarget = "";
$project = "";
$cvs = 0;

while ( @ARGV ) {				# parse command line args
    $_ = shift @ARGV;
    if ( s/^-// ) {
	if ( $_ eq "app" ) {
	    $libtarget = "";
	} elsif ( $_ eq "cvs" ) {
	    $cvs = 1;
	} elsif ( $_ eq "lib" ) {
	    $libtarget = "LIBTARGET = 1\n";
	} else {
	    die "Invalid option";
	}
    } else {
	$project = $_;
    }
}

$project || die "No project name specified";

$win32make =
"#############################################################################
# \$Id: //depot/qt/main/bin/makeqtmake.bat#1 $
#
# Win32 Makefile, requires Microsoft nmake.
#
# Copyright (C) 1998 by Troll Tech AS.  All rights reserved.
#
#############################################################################

PROJECT = $project
$libtarget
!INCLUDE \$(QTDIR)\\Makefile.inc
";

$unixmake =
"#############################################################################
# \$Id: //depot/qt/main/bin/makeqtmake.bat#1 $
#
# Unix Makefile, requires GNU make (gmake).
#
# Copyright (C) 1998 by Troll Tech AS.  All rights reserved.
#
#############################################################################

PROJECT = $project
$libtarget
include \$(QTDIR)/GNUmakefile.inc
";

if ( open( F, "> Makefile" ) ) {
    print F $win32make;
    close F;
}
if ( open( F, "> GNUmakefile" ) ) {
    print F $unixmake;
    close F;
}
$libtarget && ($libtarget = "-t lib");
`progen -n $project $libtarget -o $project.pro *.cpp *.h`;
$cvs && `cvs add $project.pro Makefile GNUmakefile`;
__END__
:endofperl

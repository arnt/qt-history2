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
# $Id: //depot/qt/main/bin/makeqtmake.bat#2 $
#
# Makes Qt makefiles and a project file - internal Troll Tech tool.
#
# Usage:
#    makeqtmake [-lib] [-cvs] [-nopro] projectname
#
# Copyright (C) 1998 by Troll Tech AS.  All rights reserved.
#
############################################################################

$libtarget = "";
$project = "";
$cvs = 0;
$nopro = 0;

while ( @ARGV ) {				# parse command line args
    $_ = shift @ARGV;
    if ( s/^-// ) {
	if ( $_ eq "app" ) {
	    $libtarget = "";
	} elsif ( $_ eq "cvs" ) {
	    $cvs = 1;
	} elsif ( $_ eq "lib" ) {
	    $libtarget = "LIBTARGET = 1\n";
	} elsif ( $_ eq "nopro" ) {
	    $nopro = 1;
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
# \$Id: //depot/qt/main/bin/makeqtmake.bat#2 $
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
# \$Id: //depot/qt/main/bin/makeqtmake.bat#2 $
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
if ( ! $nopro ) {
    $files = join(" ",find_files(".",'\.(h|cpp)$',0));
    `progen -n $project $libtarget -o $project.pro $files`;
}
$cvs && `cvs add $project.pro Makefile GNUmakefile`;
exit 0;



#
# Finds files.
#
# Examples:
#   find_files("/usr","\.cpp$",1)   - finds .cpp files in /usr and below
#   find_files("/tmp","^#",0)	    - finds #* files in /tmp
#

sub find_files {
    my($dir,$match,$descend) = @_;
    my($file,$p,@files);
    local(*D);
    $dir =~ s=\\=/=g;
    ($dir eq "") && ($dir = ".");
    if ( opendir(D,$dir) ) {
	if ( $dir eq "." ) {
	    $dir = "";
	} else {
	    ($dir =~ /\/$/) || ($dir .= "/");
	}
	foreach $file ( readdir(D) ) {
	    next if ( $file  =~ /^\.\.?$/ );
	    $p = $dir . $file;
	    ($file =~ /$match/) && (push @files, $p);
	    if ( $descend && -d $p && ! -l $p ) {
		push @files, &find_files($p,$match,$descend);
	    }
	}
	closedir(D);
    }
    return @files;
}
__END__
:endofperl

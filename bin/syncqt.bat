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
perl.exe -S syncqt.bat %ARGS%
goto endofperl
@rem ';
#!/usr/bin/perl
############################################################################
# $Id: //depot/qt/main/bin/syncqt.bat#4 $
#
# Synchronizes Qt header files - internal Troll Tech tool.
#   - Creates symlinks on Unix.
#   - Copies files on Windows.
#
# Copyright (C) 1997-1998 by Troll Tech AS.  All rights reserved.
#
############################################################################

die "syncqt: QTDIR not defined" if ! $ENV{"QTDIR"};
$fast=0;

$basedir = $ENV{"QTDIR"};
$includedir = $basedir . "/include";

while ( $#ARGV >= 0 ) {
    if ( $ARGV[0] eq "-fast" ) {
	$fast=1;
    } elsif ( $ARGV[0] eq "-inc" ) {
	shift;
	$includedir = $ARGV[0];
	if ( $includedir =~ /^\// ) {
	    $includedir = `pwd` ."/". $includedir;
	}
    }
    shift;
}

undef $/;

@dirs = ( "src/tools",
	  "src/kernel",
	  "src/widgets",
	  "src/dialogs",
	  "src/compat",
	  "extensions/xt/src",
	  "extensions/opengl/src",
	  "extensions/nsplugin/src",
	  "extensions/imageio/src" );

foreach $p ( @dirs ) {
    chdir "$basedir/$p";
    @ff = find_files(".",'\.h$',0);
    foreach ( @ff ) { $_ = "$p/$_"; }
    push @files, @ff;
}

if ( check_unix() ) {
    chdir $includedir;
    foreach $f ( @files ) {
	$h = $f;
	$h =~ s-.*/--g;
	if ( ! -l $h ) {
	    symlink("../" . $f, $h);
	    print "symlink created for src/$f\n";
	}
    }
} else {
    foreach $f ( @files ) {
	$h = $f;
	$h =~ s-.*/--g;
	sync_files("$basedir/$f", "$includedir/$h", $fast);
    }
}

exit 0;


#
# sync_files(file,ifile)
#
# If ifile does not exist, file is copied to ifile, otherwise
# the newest file is copied over the older file.
#

sub sync_files()
{
    my ($file,$ifile,$fast,$copy,$knowdiff,$filecontents,$ifilecontents) = @_;

    if ( $fast ) {
	# Uni-directional synchronization
	if ( (stat($ifile))[9] < (stat($file))[9] ) {
	    open( I, "< " . $file ) || die "Could not open $file for reading";
	    $filecontents = <I>;
	    close I;
	    $copy = -1;
	    $knowdiff = 1;
	}
    } else {
	# Bi-directional synchronization
	open( I, "< " . $file ) || die "Could not open $file for reading";
	$filecontents = <I>;
	close I;
	if ( open(I, "< " . $ifile) ) {
	    $ifilecontents = <I>;
	    close I;
	    $copy = (stat($ifile))[9] <=> (stat($file))[9];
	    $knowdiff = 0,
	} else {
	    $copy = -1;
	    $knowdiff = 1;
	}
    }

    if ( $knowdiff || ($filecontents ne $ifilecontents) ) {
	if ( $copy > 0 ) {
	    open(O, "> " . $file) || die "Could not open $file for writing";
	    print O $ifilecontents;
	    close O;
	    print "$file written\n";
	} elsif ( $copy < 0 ) {
	    open(O, "> " . $ifile) || die "Could not open $ifile for writing";
	    print O $filecontents;
	    close O;
	    print "$ifile written\n";
	}
    }
}


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


#
# check_unix()
#
# Returns 1 if this is a Unix, 0 otherwise.
#

sub check_unix {
    my($r);
    $r = 0;
    if ( -f "/bin/uname" ) {
	$r = 1;
	(-f "\\bin\\uname") && ($r = 0);
    }
    return $r;
}
__END__
:endofperl

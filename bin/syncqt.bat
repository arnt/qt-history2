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
# $Id: $
#
# Synchronizes Qt header files - internal Trolltech tool.
#   - Creates symlinks on Unix.
#   - Copies files on Windows.
#
# Copyright (C) 1997-1998 by Trolltech AS.  All rights reserved.
#
############################################################################

die "syncqt: QTDIR not defined" if ! $ENV{"QTDIR"};
$fast=0;
$force_win=0;

$basedir = $ENV{"QTDIR"};
$includedir = $basedir . "/include";
$privatedir = $basedir . "/include/private";

while ( $#ARGV >= 0 ) {
    if ( $ARGV[0] eq "-fast" ) {
	$fast=1;
    } elsif ( $ARGV[0] eq "-inc" ) {
	shift;
	$includedir = $ARGV[0];
	if ( $includedir =~ /^\// ) {
	    $includedir = `pwd` ."/". $includedir;
	}
    } elsif ( $ARGV[0] eq "-windows" ) {
	$force_win=1;
    }
    shift;
}

undef $/;

opendir SRC, "$basedir/src";
@dirs = map { -d "$basedir/src/$_" ? "src/$_" : () } readdir(SRC);
closedir SRC;
@dirs = ( @dirs, "extensions/xt/src", "extensions/nsplugin/src" );

$specdir = "mkspecs/" . $ENV{"MKSPEC"};
@dirs = ( @dirs, $specdir );

foreach $p ( @dirs ) {
    if ( -d "$basedir/$p" ) {
	chdir "$basedir/$p";
	@ff = find_files( ".", "^[-a-z0-9]*(?:_[^p].*)?\\.h\$" , 0 );
	@pf = find_files( ".", "^[-a-z0-9]*[_][p]\\.h\$" , 0 );
	foreach ( @ff ) { $_ = "$p/$_"; }
	foreach ( @pf ) { $_ = "$p/$_"; }
	push @files, @ff;
	push @pfiles, @pf;
    }
}

if ( check_unix() ) {
    chdir $includedir;
    foreach $f ( @files ) {
	$h = $f;
	$h =~ s-.*/--g;
	if ( ! -l $h ) {
	    symlink("../" . $f, $h);
	    print "symlink created for $f\n";
	}
    }
    chdir $privatedir;
    foreach $f ( @pfiles ) {
	$h = $f;
	$h =~ s-.*/--g;
	if ( -l $h && ! -f $h ) {
	    unlink $h;
	}
	if ( ! -l $h ) {
	    symlink($basedir . "/" . $f, $h);
	    print "symlink created for $f\n";
	}
    }
} else {
    mkdir $includedir, 0777;
    mkdir $privatedir, 0777;
    foreach $f ( @files ) {
	$h = $f;
	$h =~ s-.*/--g;
	sync_files("$basedir/$f", "$includedir/$h", $fast);
    }
    foreach $f ( @pfiles ) {
	$h = $f;
	$h =~ s-.*/--g;
	sync_files("$basedir/$f", "$privatedir/$h", $fast);
    }
    sync_files("$basedir/dist/win/Makefile", "$basedir/Makefile", $fast);
    sync_files("$basedir/dist/win/bin/moc.exe", "$basedir/bin/moc.exe", $fast);
    sync_files("$basedir/dist/win/bin/configure.exe", "$basedir/bin/configure.exe", $fast);
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
	    binmode I;
	    $filecontents = <I>;
	    close I;
	    $copy = -1;
	    $knowdiff = 1;
	}
    } else {
	# Bi-directional synchronization
	open( I, "< " . $file ) || die "Could not open $file for reading";
	binmode I;
	$filecontents = <I>;
	close I;
	if ( open(I, "< " . $ifile) ) {
	    binmode I;
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
	    binmode O;
	    print O $ifilecontents;
	    close O;
	    print "$file written\n";
	} elsif ( $copy < 0 ) {
	    open(O, "> " . $ifile) || die "Could not open $ifile for writing";
	    binmode O;
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
    if ( $force_win != 0) {
	return 0;
    }
    if ( -f "/bin/uname" ) {
	$r = 1;
	(-f "\\bin\\uname") && ($r = 0);
    }
    return $r;
}
__END__
:endofperl

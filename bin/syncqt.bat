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
#!/usr/bin/perl -w
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

use Config;
use strict;

die "syncqt: QTDIR not defined" if ! $ENV{"QTDIR"};
my $fast=0;
my $force_win=0;
my $force_relative=0;
$force_relative=1 if ( -d "/System/Library/Frameworks" );

my $basedir = $ENV{"QTDIR"};
my $includedir = $basedir . "/include";
my $privatedir = $basedir . "/include/private";

my $showonly = undef;
my $showonlypriv = undef;

while ( $#ARGV >= 0 ) {
    if ( $ARGV[0] eq "-fast" ) {
	$fast=1;
    } elsif ( $ARGV[0] eq "-inc" ) {
	shift;
	$includedir = $ARGV[0];
	if ( $includedir !~ /^\// ) {
	    $includedir = `pwd`;
	    chomp $includedir;
	    $includedir .= "/". $includedir;
	}
	$privatedir = $includedir . "/private";
    } elsif ( $ARGV[0] eq "-show" ) {
	$showonly++;
    } elsif ( $ARGV[0] eq "-showpriv" ) {
	$showonlypriv++;
    } elsif ( $ARGV[0] eq "-windows" ) {
	$force_win=1;
    } elsif ( $ARGV[0] eq "-relative" ) {
        $force_relative=1;
    }
    shift;
}

undef $/;

mkdir $includedir, 0777;
mkdir $privatedir, 0777;

my @ignore_files = ( "qconfig.h", "qmodules.h", "qttableview.h", "qtmultilineedit.h" );

opendir SRC, "$basedir/src";
my @dirs = map { -d "$basedir/src/$_" ? "src/$_" : () } readdir(SRC);
closedir SRC;
@dirs = ( @dirs, "extensions/xt/src", "extensions/nsplugin/src", "extensions/activeqt/control", "extensions/activeqt/container", "extensions/motif/src", "tools/designer/uilib", "tools/assistant/lib" );

push @dirs, "mkspecs/" . $ENV{"MKSPEC"} if defined $ENV{"MKSPEC"}; 

my @files;
my @pfiles;
my $p;
foreach $p ( @dirs ) {
    if ( -d "$basedir/$p" ) {
	chdir "$basedir/$p";
	my @ff = find_files( ".", "^[-a-z0-9]*(?:_[^p]*)?\\.h\$" , 0 );
	my @pf = find_files( ".", "^[-a-z0-9_]*[_][p]\\.h\$" , 0 );
	foreach ( @ff ) { 
	  my $file = "$p/$_"; 
	  my $h = $file; 
	  $h =~ s-.*/--g;
	  
	  foreach ( @ignore_files ) { 
	    if( "$h" eq "$_" ) {
	      $file = 0;
	    }
	  }
	  if( "$file" ) {
	    push @files, $file
	  }
	}
	foreach ( @pf ) { 
	  my $file = "$p/$_"; 
	  my $h = $file;
	  $h =~ s-.*/--g;
	  
	  foreach ( @ignore_files ) { 
	    if( "$h" eq "$_" ) {
	      $file = 0;
	    }
	  }
	  if( "$file" ) {
	    push @pfiles, $file
	  }
	}
    }
  }

if ( $showonly ) {
    foreach ( @files ) {
	print "$_\n";
    }
    exit( 0 );
}
if ( $showonlypriv ) {
    foreach ( @pfiles ) {
	print "$_\n";
    }
    exit( 0 );
}

if ( check_unix() ) {
    my $f;
    chdir $includedir;
    foreach $f ( @files ) {
	my $h = $f;
	$h =~ s-.*/--g;
	if ( -l $h && ! -f $h ) {
	    unlink $h;
	}
	if ( ! -l $h ) {
	  symlink_files($basedir . "/" . $f, $h);
	}
    }
    chdir $privatedir;
    foreach $f ( @pfiles ) {
	my $h = $f;
	$h =~ s-.*/--g;
	if ( -l $h && ! -f $h ) {
	    unlink $h;
	}
	symlink_files($basedir . "/" . $f, $h) if ( ! -l $h );
    }
} else {
    my $f;
    foreach $f ( @files ) {
	my $h = $f;
	$h =~ s-.*/--g;
	sync_files("$basedir/$f", "$includedir/$h", $fast);
    }
    foreach $f ( @pfiles ) {
	my $h = $f;
	$h =~ s-.*/--g;
	sync_files("$basedir/$f", "$privatedir/$h", $fast);
    }
    sync_files("$basedir/dist/win/Makefile", "$basedir/Makefile", $fast);
    sync_files("$basedir/dist/win/bin/moc.exe", "$basedir/bin/moc.exe", $fast);+
    sync_files("$basedir/dist/win/bin/configure.exe", "$basedir/bin/configure.exe", $fast);
}

exit 0;


#
# symlink_files(file,ifile)
#
# file is symlinked to ifile
#

sub symlink_files
{
  my ($file, $ifile) = @_;
  print "symlink created for $file ";
  if ( $force_relative ) {
    my $t = `pwd`; 
    my $c = -1; 
    my $p = "../";
    $t =~ s-^$basedir/--;
    $p .= "../" while( ($c = index( $t, "/", $c + 1)) != -1 );
    $file =~ s-^$basedir/-$p-;
    print " ($file)\n"
  }
  print "\n";
  symlink($file, $ifile);
}

#
# sync_files(file,ifile)
#
# If ifile does not exist, file is copied to ifile, otherwise
# the newest file is copied over the older file.
#

sub sync_files
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
    $r = 1;
    if ( $force_win != 0) {
	return 0;
    }
    $_ = $Config{'osname'};
    $r = 0 if( /(ms)|(cyg)win/i );
    return $r;
}
__END__
:endofperl

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
#
# Copyright (C) 1997-%THISYEAR% by Trolltech AS.  All rights reserved.
#
############################################################################

use File::Basename;
use Config;
use strict;

#required
die "syncqt: QTDIR not defined" if ! $ENV{"QTDIR"};

#global
my $basedir = $ENV{"QTDIR"};
my $includedir = $basedir . "/include";

#options
my $showonly = 0;
my $force_win=0;
my $force_relative=0;
$force_relative=1 if ( -d "/System/Library/Frameworks" );

while($#ARGV >= 0) {
    if($ARGV[0] eq "-show") {
	$showonly++;
    } elsif($ARGV[0] eq "-outdir" || $ARGV[0] eq "-inc") {
	shift;
	my $outdir = $ARGV[0];
	if($outdir !~ /^\//) {
	    $includedir = `pwd`;
	    chomp $includedir;
	    $includedir .= "/" . $outdir;
	} else {
	    $includedir = $outdir;
	}
    } elsif($ARGV[0] eq "-windows") {
	$force_win=1;
    }
    shift;
}

$basedir =~ s=\\=/=g;
$includedir =~ s=\\=/=g;

undef $/;

mkdir $includedir, 0777;

my @ignore_headers = ();
my @ignore_master_headers = ( "qt.h", "qpaintdevicedefs.h" );

my %dirs = (
	 "src/gui" => "QtGUI",
	 "src/opengl" => "QtOpenGL",
	 "src/core" => "QtCore",
	 "src/xml" => "QtXML",
	 "src/sql" => "QtSQL",
	 "src/network" => "QtNetwork",
	 "src/canvas" => "QtCanvas",
	 "src/compat" => "Qt3Compat",
);
$dirs{"mkspecs" . $ENV{"MKSPEC"}} = "QCore" if defined $ENV{"MKSPEC"}; 

my $dir;
foreach $dir (keys %dirs) {
    my $lib = $dirs{$dir};

    #calc subdirs
    my @subdirs = ($dir);
    foreach (@subdirs) {
	my $subdir = "$_";
	opendir DIR, "$subdir";
	while(my $t = readdir(DIR)) {
	    push @subdirs, "$subdir/$t" if(-d "$subdir/$t" && !($t eq ".") && !($t eq "..") && !($t eq "arch"));
	}
	closedir DIR;
    }

    #calc files and "copy" them
    my $master_header_out;
    foreach (@subdirs) {
	my $subdir = "$_";
	my @headers = find_files("$subdir", "^[-a-z0-9_]*\\.h\$" , 0);
	foreach (@headers) { 
	    my $header = "$_"; 
	    foreach (@ignore_headers) { 
		$header = 0 if("$header" eq "$_");
	    }
	    if("$header") {
		if($showonly) {
		    print "$header [$lib]\n";
		} else {
		    #figure out if this belongs in the master file
		    unless($header =~ /_/) {
			my $public_header = $header;
			foreach (@ignore_master_headers) { 
			    $public_header = 0 if("$header" eq "$_");
			}
			$master_header_out .= "#include \"$public_header\"\n" if($public_header);
		    }
		    #now sync the header file (ie symlink/include it)
		    sync_header($header, $basedir . "/" . $subdir . "/" . $header, $lib);
		}
	    }
	}
    }

    #finally generate the "master" include file
    my $master_include = "$includedir/$lib/$lib";
    if(-e "$master_include") {
	open MASTERINCLUDE, "<$master_include";
	binmode MASTERINCLUDE;
	my $oldmaster = <MASTERINCLUDE>;
	close MASTERINCLUDE;
	$master_include = 0 if($oldmaster eq $master_header_out);
    }
    if($master_include) {
	print "header (master) created for $lib\n";
	open MASTERINCLUDE, ">$master_include";
	print MASTERINCLUDE "$master_header_out";
	close MASTERINCLUDE;
    }
}


unless(!check_unix()) {
   symlink_file("$basedir/dist/win/Makefile", "$basedir/Makefile");
   symlink_file("$basedir/dist/win/Makefile.win32-g++", "$basedir/Makefile.win32-g++");
}

exit 0;

#
# sync_header(header, iheader, library)
#
# header is synconized to iheader
#
sub sync_header {
    my ($header, $iheader, $library) = @_;
    $iheader =~ s=\\=/=g;
    $header =~ s=\\=/=g;
    if(-e "$iheader") {
	my $iheader_no_basedir = $iheader;
	$iheader_no_basedir =~ s,^$basedir/?,,;

	my $headers_created = 0;
	my @headers_out;
	if (($header =~ /_p.h$/)) {
	    @headers_out = ( "$includedir/Qt/private/$header", "$includedir/$library/private/$header" );
	} else {
	    @headers_out = ( "$includedir/Qt/$header", "$includedir/$library/$header" );
	}
	unlink "$includedir/$header" if(-e "$includedir/$header"); #remove old symlink from 3.x
	foreach(@headers_out) { 
	    my $header_out = $_;
	    if(!-e "$header_out") {
		$headers_created++;

		my $header_out_dir = dirname($header_out);
		mkdir $header_out_dir, 0777;

		my $iheader_out = $iheader;
		my $iheader_out_dir = dirname($iheader_out);

		#relativification
		my $match_dir = 0;
		for(my $i = 1; $i < length($iheader_out_dir); $i++) {
		    my $tmp = substr($iheader_out_dir, 0, $i);
		    last unless($header_out_dir =~ /^$tmp/);
		    $match_dir = $tmp;
		}
		if($match_dir) {
		    my $after = substr($header_out_dir, length($match_dir));
		    my $count = ($after =~ tr,/,,);
		    my $dots = "";
		    for(my $i = 0; $i <= $count; $i++) {
			$dots .= "../";
		    }
		    $iheader_out =~ s,^$match_dir,$dots,;
		}

		#write it
		open HEADER, ">$header_out" || die "Could not open $header_out for writing!\n";
		print HEADER "#include \"$iheader_out\"\n";
		close HEADER;
	    }
	}
	print "header created for $iheader_no_basedir\n" if($headers_created > 0);
    }
}

#
# symlink_file(file,ifile)
#
# file is symlinked to ifile (or copied if filesystem doesn't support symlink)
#
sub symlink_file
{
    my ($file,$ifile, $fast,$copy,$knowdiff,$filecontents,$ifilecontents) = @_;

    if (check_unix()) {
	print "symlink created for $file ";
	if ( $force_relative && ($ifile =~ /^$basedir/)) {
	    my $t = `pwd`; 
	    my $c = -1; 
	    my $p = "../";
	    $t =~ s-^$basedir/--;
	    $p .= "../" while( ($c = index( $t, "/", $c + 1)) != -1 );
	    $file =~ s-^$basedir/-$p-;
	    print " ($file)\n";
	}
	print "\n";
	symlink($file, $ifile);
	return;
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
	    $p = $file;
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
    } elsif ( -f "/usr/bin/uname" ) {
        $r = 1;
	(-f "\\usr\\bin\\uname") && ($r = 0);
    }
    if($r) {
	$_ = $Config{'osname'};
	$r = 0 if( /(ms)|(cyg)win/i );
    }
    return $r;
}
__END__
:endofperl

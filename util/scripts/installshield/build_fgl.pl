#!/usr/bin/perl
#
# Builds an InstallShield file list.
#
# Copyright (c) 1998 by Troll Tech AS. All rights reserved.
#

$recurse = 0;
$top = "";
$dir = "";

while ( @ARGV ) {				# parse command line args
    $_ = shift @ARGV;
    if ( s/^-// ) {
	if ( /^t(.*)/ ) {
	    $top = ($1 eq "") ? shift @ARGV : $1;
	    $top = fix_path($top);
	} elsif ( /^o(.*)/ ) {
	    $outfile = ($1 eq "") ? shift @ARGV : $1;
	    ($outfile eq "-") && ($outfile = "");
	} elsif ( /^r$/ ) {
	    $recurse = 1;
	} else {
	    &usage();
	}
    } else {
	$dir &&
	    &error("You can only specify one directory");
	$dir = fix_path($_);
    }
}

$dir || &usage;

$outfile eq "" || open(STDOUT,">" . fix_path($outfile)) ||
    &error("Can't create \"$outfile\"");

if ( $top ) {
    @t = split(/\\/,$top);
    $d = "";
    for ( @t ) {
	if ( $d eq "" ) {
	    print "[TopDir]\nSubDir0=$_\n\n";
	    $d = $_;
	} else {
	    print "[$d]\nSubDir0=$d\\$_\n";
	    print "fulldirectory=" . fix_path($dir . "\\" . $d) . "\n\n";
	    $d .= "\\$_";
	}
    }
}

process_dir($top,$dir);
print "[General]\nType=FILELIST\nVersion=1.00.000\n";
exit 0;

sub process_dir {
    my($top,$dir) = @_;
    my($top2,$dir2,$out,$i,@f,@d);
    if ( $top ) {
	$top2 = $top . "\\";
	$dir2 = $dir . "\\" . $top;
    } else {
	$top2 = $top;
	$dir2 = $dir;
    }
    @f = find_files($dir2,".",0);
    $out = "";
    $i = 0;
    for ( @f ) {
	$out .= "file$i=$_\n";
	$i++;
    }
    if ( $recurse ) {
	chdir($dir2);
	@d = find_dirs(".",".",0);
	$i = 0;
	for ( @d ) {
	    $out .= "SubDir$i=" . $top2 . "$_\n";
	    $i++;
	}
    }
    if ( $top ) {
	print "[$top]\n${out}fulldirectory=$dir2\n\n";
    } else {
	print "[TopDir]\n$out\n";
    }
    if ( @d ) {
	for ( @d ) {
	    process_dir( $top2 . $_, $dir );
	}
    }
}


#
# usage()
#
# Prints a message about program usage and exits
#

sub usage {
    print STDERR "Usage:\n    build_fgl [options] directory\n";
    print STDERR "Options:\n";
    print STDERR "    -o file    Write output to file\n";
    print STDERR "    -r         Recurse directories\n";
    print STDERR "    -t dir     Specify top level directory\n";
    exit 1;
}


#
# error(msg)
#
# Prints the message and exits
#

sub error {
    my($msg) = @_;
    print STDERR "build_fgl error: " . $msg . "\n";
    exit 1;
}


#
# Finds files.
#

sub find_files {
    my($dir,$match,$descend) = @_;
    my($file,$p,@files);
    local(*D);
    $dir =~ s=\\=/=g;
    ($dir eq "") && ($dir = ".");
    if ( opendir(D,fix_path($dir)) ) {
	if ( $dir eq "." ) {
	    $dir = "";
	} else {
	    ($dir =~ /\/$/) || ($dir .= "/");
	}
	foreach $file ( readdir(D) ) {
	    next if ( $file  =~ /^\.\.?$/ );
	    $p = fix_path($dir . $file);
	    (-f $p) && ($file =~ /$match/) && (push @files, $p);
	    if ( $descend && -d $p && ! -l $p ) {
		push @files, &find_files($p,$match,$descend);
	    }
	}
	closedir(D);
    }
    return @files;
}


#
# Finds directories.
#

sub find_dirs {
    my($dir,$match,$descend) = @_;
    my($file,$p,@files);
    local(*D);
    $dir =~ s=\\=/=g;
    ($dir eq "") && ($dir = ".");
    if ( opendir(D,fix_path($dir)) ) {
	if ( $dir eq "." ) {
	    $dir = "";
	} else {
	    ($dir =~ /\/$/) || ($dir .= "/");
	}
	foreach $file ( readdir(D) ) {
	    next if ( $file  =~ /^\.\.?$/ );
	    $p = fix_path($dir . $file);
	    (-d $p) && ($file =~ /$match/) && (push @files, $p);
	    if ( $descend && -d $p && ! -l $p ) {
		push @files, &find_files($p,$match,$descend);
	    }
	}
	closedir(D);
    }
    return @files;
}


#
# fix_path(path)
#
# Converts all '\' to '/' if this really seems to be a Unix box.
#

sub fix_path {
    my($p) = @_;
    if ( $really_unix ) {
	$p =~ s-\\-/-g;
    } else {
	$p =~ s-/-\\-g;
    }
    return $p;
}

#!/usr/bin/perl

use strict;

# global variables
my %qtsettings;

# global variables (modified by options)
my $qtconf = "/etc/qt.conf";
my $version = 0;
my $conf_variable = 0;
my $conf_value = 0;

sub showUsage
{
    exit 0;
}

#parse commandline
while ( @ARGV ) {
    my $var = 0;
    my $val = 0;

    #parse
    my $arg = shift @ARGV;
    if ("$arg" eq "-qtconf") {
	$var = "qtconf";
	$val = shift @ARGV;
    } elsif ("$arg" eq "-value") {
	$var = "value";
	$val = shift @ARGV;
    } elsif ("$arg" eq "-version") {
	$var = "version";
	$val = shift @ARGV;
    } elsif (!("$arg" =~ "/^-/")) {
	$var = "variable";
	$val = "$arg";
    } 

    #do something
    if(!$var || "$var" eq "show_help") {
	print "Unknown option: $arg\n\n" if(!$var);
	showUsage();
    } elsif("$var" eq "value") {
	die "Too many values!" if($conf_value);
	$conf_value = "$val";
	$conf_value =~ s/\/\/+/\//g;
    } elsif("$var" eq "variable") {
	die "Too many variables!" if($conf_variable);
	$conf_variable = "$val";
    } elsif("$var" eq "qtconf") {
	$qtconf = "$val";
    } elsif("$var" eq "version") {
	$version = "$val";
    }
}
die "No variable requested!" if(!$conf_variable);

#read in the settings
if(open(SETTINGS, "<$qtconf")) {
    my $section = 0;
    while(<SETTINGS>) {
	my $line = "$_";
	if("$line" =~ /\[(.*)\]/) {
	    $section = $1;
	} elsif ($section && "$line" =~ /^(.*) *= *(.*)$/) {
	    my $val = $2;
	    my $var = $section . "/" . "$1";
	    $var =~ s/ *$//;
	    $qtsettings{$var} = $val;
	} else {
	    die "Cannot parse $qtconf [$line]";
	}
	chomp;
    }
    close(SETTINGS);
}

#process request
if($conf_value) {
    my $path = "Paths/";
    $path = $path . $version . "/" if($version);
    $qtsettings{$path . $conf_variable} = "$conf_value";
    if(open(SETTINGS, ">$qtconf")) {
	my $section = 0;
	foreach (keys(%qtsettings)) {
	    my $var = $_;
	    my $val = "$qtsettings{$var}";
	    if($var = /(.*)\/([^\/]*)/) {
		my $var_s = $1;
		$var = $2;
		if(!$section || !($var_s eq $section)) {
		    $section = $var_s;
		    print SETTINGS "[$section]\n";
		}
	    }
	    print SETTINGS "$var = $val\n";
	}
	close(SETTINGS);
    }
} else {
    my $path = "Paths/";
    $path = $path . $version . "/" if($version);
    my $val = $qtsettings{$path . $conf_variable};
    print("$conf_variable = $val\n");
}


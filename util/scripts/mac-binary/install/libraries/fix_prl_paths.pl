#!/usr/bin/perl

use strict;

open(OLDFILE, "<" . "$ARGV[0]") || die "Couldn't open input file: $ARGV[0]";
open(NEWFILE, ">" . "$ARGV[1]") || die "Couldn't open output file: $ARGV[1]";

while (my $line = <OLDFILE>) {
    $line =~ s,-L/(\w+(\w|\s|/)*)/pgsql/lib,,g;
    $line =~ s,-L/(\w+(\w|\s|/)*)/mysql/lib,,g;
    $line =~ s,-F/private/tmp/qt-stuff/source/qt-mac-\w+-src-4\.\d\.\d/lib,,g;
    $line =~ s,-L/private/tmp/qt-stuff/source/qt-mac-\w+-src-4\.\d\.\d/lib,,g;
    print NEWFILE $line;
}

close OLDFILE;
close NEWFILE;

#!/usr/bin/perl

my $QTDIR_INCLUDE = "$ENV{QTDIR}/include";
my @FIXDIRS = ();
my $single_include = 0;

while ( $#ARGV >= 0 ) {
    if ( $ARGV[0] eq "-single" ) {
	$single_include++;
    } else {
	push @FIXDIRS, $ARGV[0];
    }
    shift;
}

my @INCLUDE_SUBS;
if (opendir(INC, "$QTDIR_INCLUDE")) {
    foreach $p (readdir(INC)) {
	push @INCLUDE_SUBS, $p if(-d "$QTDIR_INCLUDE/$p" && !($p eq "Qt") && !($p eq ".") && !($p eq ".."));
    }
    closedir(INC);
}

sub clean_files {
    ($dir) = @_;
    $dir =~ s=\\=/=g;
    $dir = "." if($dir eq "");
    my @subdirs;
    if (opendir(D,$dir)) {
	if ($dir eq ".") {
	    $dir = "";
	} else {
	    ($dir =~ /\/$/) || ($dir .= "/");
	}
	foreach $p (readdir(D)) {
	    next if($p  =~ /^\.\.?$/);
	    my $file = $dir . $p;
	    if(!-l $file) {
		if (-d $file) {
		    push @subdirs, $file;
		} elsif($file =~ m/\.cpp$/ || $file =~ m/\.h$/ || $file =~ m/\.l$/ || $file =~ m/\.y$/) {
		    if(open(F, "<$file")) {
			%subdirs = ();
			my $file_out = "${file}.tmp";
			unlink $file_out;
			if(open(F_OUT, ">$file_out")) {
			    while($line = <F>) {
				my $output_line = 1;
				if($line =~ /^ *\# *include/) {
				    my $fix_quoted = 0;
				    my $file = $line;
				    chomp $file;
				    $fix_quoted++ if(!($file =~ s,^.*<(.*)>.*$,$1,) && $file =~ s,^.*"(.*)".*$,$1,);
				    foreach $incdir (@INCLUDE_SUBS) {
					if(-e "$QTDIR_INCLUDE/$incdir/$file") {
					    my $outfile = "$incdir/$file";
					    unless(defined $subdirs{$incdir}) {
						$subdirs{$incdir} = 1;
						$outfile = "$incdir/$incdir" if($single_include);
					    } elsif($single_include) {
						$output_line = 0;
					    }
					    if($fix_quoted) {
						$line =~ s,"$file",<$outfile>,;
					    } else {
						$line =~ s,$file,$outfile,;
					    }
					    last;
					}
				    }
				}
				print F_OUT "$line" if($output_line);
			    }
			    close(F_OUT);
			    close(F);
			    if(keys(%subdirs) > 0) {
				print "Cleaning [$file]....\n";
				if(!unlink $file) {
				    system("p4 edit $file");
				    unlink $file;
				}
				link $file_out, $file;
			    }
			    unlink $file_out;
			} else {
			    close(F);
			}
		    }
		} 
	    }
	}
	closedir(D);
	foreach $dir (@subdirs) {
	    clean_files($dir);
	}
    } else {
	print "Failure!!!!!!!!!!! $dir\n";
    }
}
foreach $dir (@FIXDIRS) {
    clean_files $dir;
}

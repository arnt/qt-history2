#!/usr/bin/perl -w
#./exports.pl -name QtCore -o exports $QTDIR/src/core -symbol Q_CORE_EXPORT -I$QTDIR/include/QtCore/ -I$QTDIR/mkspecs/default -I$QTDIR/src/core/.moc/debug-shared

use strict;
use File::Basename;

my $EXPORT_OUT = "-"; #output
my @EXPORT_INPUTS; #files to read (or directories)
my $EXPORT_NAME = "Qt"; #name in the linker script
my $EXPORT_SYMBOL = 0; #symbol to look for in files
my $EXPORT_CPP_ARGS = ""; #-D/-I to use in processing files
my $EXPORT_FORMAT = "version-script";

sub showHelp {
    print "$0: We can operate in two different modes, one uses a heuristic to determine\n";
    print "    the exported symbols, the other (see -symbol below) will actually run the\n";
    print "    preprocessor over input files, this is much slower, and unfortunetely bound\n";
    print "    to the current configuration. Therefore the default is to use the heuristic.\n";
    print "\n";
    print "-help               This help.\n";
    print "-o <output>         Writes data to output, defaults to standard out\n";
    print "-symbol <symbol>    Uses symbol as the exact exported symbol (ie, Q_GUI_EXPORT)\n";
    print "-D<define>          If -symbol is used define is passed onto the preprocessor.\n";
    print "-I<include>         If -symbol is used include is added to the include path.\n";
    print "-format <format>    Determines the output format (version-script is the default)\n";
    print "-name <name>        Some output formats will require a base symbol name (version-script)\n";
    print "\n";
    print "Any other arguments are processed as input directories (or files) to look for symbols\n";
    print "in.\n";
}

while($#ARGV >= 0) {
    if($ARGV[0] eq "-o") {
	shift;
	$EXPORT_OUT = $ARGV[0];
    } elsif($ARGV[0] eq "-symbol") {
	shift;
	$EXPORT_SYMBOL = $ARGV[0];
    } elsif($ARGV[0] eq "-format") {
	shift;
	$EXPORT_FORMAT = $ARGV[0];
    } elsif($ARGV[0] eq "-name") {
	shift;
	$EXPORT_NAME = $ARGV[0];
    } elsif($ARGV[0] =~ /^-D/ || $ARGV[0] =~ /^-I/) {
	$EXPORT_CPP_ARGS .= "$ARGV[0] ";
    } elsif($ARGV[0] eq "-h" || $ARGV[0] eq "-help") {
        showHelp();
        exit 0;
    } elsif($ARGV[0] =~ /^-/) {
        print "Unknown argument $ARGV[0]!\n";
        showHelp();
	exit 1;
    } else {
        print "$ARGV[0] does not exist!\n" unless(-e "$ARGV[0]");
	push @EXPORT_INPUTS, $ARGV[0];
    }
    shift;
}
($#EXPORT_INPUTS == -1) && die "$0 [options] inputs\n";
my $EXPORT_PREPROCESS = 1;
if(!$EXPORT_SYMBOL) {
   $EXPORT_PREPROCESS = 0;
   $EXPORT_SYMBOL = "Q[^ ]*(?:_[^ ]*)?_EXPORT(?:_[^ ]*)?";
   print "ARGUMENTS TO PREPROCESSOR IGNORED WITHOUT SYMBOL (-symbol)!\n" if(length($EXPORT_CPP_ARGS));
}

#symbol lookup
my %CLASSES=();
my %GLOBALS=();
sub find_exports {
    my $ret = 0;
    my ($file) = @_;
    return if(!($file =~ /\.(h|cpp)$/) || basename($file) =~ /^moc_/);
    return if($file =~ /_(win|wce)\.(h|cpp)$/ || basename($file) eq "qt_windows.h"); #no windows fu

    my $parsable = "";
    if($EXPORT_PREPROCESS) {
        if(!open(F, "g++ -E -D${EXPORT_SYMBOL}=${EXPORT_SYMBOL} $EXPORT_CPP_ARGS $file |")) {
	    print "Could not open $file in the preprocessor!\n";
	    return 0;
        }
    } else {
        if(!open(F, "<$file")) {
	    print "Could not open $file!\n";
	    return 0;
        }
    }
    while(<F>) {
	my $line = $_;
	chomp $line;
	if($line =~ /^\#/) {
	    if($line =~ /\\$/) {
		while($line = <F>) {
		    chomp $line;
		    last unless($line =~ /\\$/);
		}
	    } else {
		$line = 0;
	    }
	}
	$line =~ s,("(\\"|[^"])*")?//.*$,, unless($EXPORT_PREPROCESS); #remove c++ comments
	$parsable .= $line if($line);
    }
    close(F);

    my $last_definition = 0;
    for(my $i = 0; $i < length($parsable); $i++) {
	my $definition = 0;
	my $character = substr($parsable, $i, 1);
	if($character eq "\"" || $character eq "'") {
	    for($i+=1; $i < length($parsable); $i++) {
		my $end = substr($parsable, $i, 1);
		if($end eq $character) {
		    $last_definition = $i+1;
		    last;
		}
	    }
	} elsif(!$EXPORT_PREPROCESS && $character eq "/" && substr($parsable, $i+1, 1) eq "*") { #I parse like this for greedy reasons
	    for($i+=2; $i < length($parsable); $i++) {
		my $end = substr($parsable, $i, 2);
		if($end eq "*/") {
		    $last_definition = $i+2;
		    $i++;
		    last;
		}
	    }
        } elsif($character eq "\"" || $character eq "'") {
	    for($i+=1; $i < length($parsable); $i++) {
		my $end = substr($parsable, $i, 1);
		if($end eq "\\") {
                    $i++;
		} elsif($end eq $character) {
		    $last_definition = $i+1;
		    $i++;
		    last;
		}
	    }
	} elsif($character eq "{") {
	    my $brace_depth = 1;
	    my $block_start = $i + 1;
	  BLOCK: for($i+=1; $i < length($parsable); $i++) {
	      my $ignore = substr($parsable, $i, 1);
	      if($ignore eq "{") {
		  $brace_depth++;
	      } elsif($ignore eq "}") {
		  $brace_depth--;
		  unless($brace_depth) {
		      for(my $i2 = $i+1; $i2 < length($parsable); $i2++) {
			  my $end = substr($parsable, $i2, 1);
			  if($end eq ";" || $end ne " ") {
			      $definition = substr($parsable, $last_definition, $block_start - $last_definition) . "}";
			      $i = $i2 if($end eq ";");
			      $last_definition = $i + 1;
			      last BLOCK;
			  }
		      }
		  }
	      }
	  }
	} elsif($character eq ";") {
	    $definition = substr($parsable, $last_definition, $i - $last_definition + 1);
	    $last_definition = $i + 1;
	}
	if($definition && $definition =~ /$EXPORT_SYMBOL /) {
	    if($definition =~ m/^ *typedef/) {
	    } elsif($definition =~ m/^ *(template *<.*> *)?(class|struct) +$EXPORT_SYMBOL +([^ ]+) ?((,|:) *(public|private) *.*)? *\{\}$/) {
		my $symbol = $3;
		$symbol =~ s/[<>+=*!-]/?/g;
		#print "2) hmm $symbol *********** $definition\n";
		$CLASSES{$symbol} = "$file" unless(!length "$symbol" || defined $CLASSES{$symbol});
	    } elsif($definition =~ /$EXPORT_SYMBOL +([a-zA-Z0-9_:\*&\(\) ]* +)?[&\*]?(([a-zA-Z0-9_]*::)?[a-zA-Z0-9_][a-zA-Z0-9_>=!\*<+_-]+)( *\(.*\)| *(\[.*\] *)?(=.*)?|)? *(;|\{\})$/) {
		my $symbol = $2;
		$symbol =~ s/[<>+=*!-]/?/g;
		#print "1) hmm $symbol *********** $1 -- $2 -- $3 -- $definition [$file]\n";
		$GLOBALS{$symbol} = "$file" unless(!length "$symbol" || defined $GLOBALS{$symbol});
	    } else {
		print "$file: dammit $definition\n";
	    }
	}
    }
    return $ret;
}

#find the symbols
sub find_files {
    my ($dir) = @_;
    ($dir = ".") if($dir eq "");

    local(*D);
    if ( opendir(D,$dir) ) {
	if ($dir eq ".") {
	    $dir = "";
	} else {
	    ($dir =~ /\/$/) || ($dir .= "/");
	}
	foreach (readdir(D) ) {
	    next if ( /^\.\.?$/ );
	    my $p = "$dir/$_";
	    if(-d "$p") {
		find_files("$p");
	    } else {
		find_exports("$p");
	    }
	}
	closedir(D);
    }
}
foreach (@EXPORT_INPUTS) {
    if(-d "$_") {
	find_files("$_");
    } else {
	find_exports("$_");
    }
}

#generate output
if("$EXPORT_OUT" eq "-") {
    open(OUTPUT, ">&STDOUT");
} else {
    open(OUTPUT, ">$EXPORT_OUT") || die "Cannot open $EXPORT_OUT!!";
}
if("$EXPORT_FORMAT" eq "version-script") {
    print OUTPUT "$EXPORT_NAME\n";
    print OUTPUT "{\n";
    print OUTPUT "  global:\n";
    print OUTPUT "  extern \"C++\"\n";
    print OUTPUT "  {";
    my $symbol_count = 0;
    foreach (keys %CLASSES) {
	my @symbols = ("${_}::*", "${_}?virtual?table", "${_}?type_info?*", "vtable?for?${_}", 
		       "non-virtual?thunk?to?${_}::*", "typeinfo?for?${_}");
	foreach (@symbols) {
	    print OUTPUT ";" if($symbol_count);
	    print OUTPUT "\n     ${_}";
	    $symbol_count++;
	}
    }
    foreach (keys %GLOBALS) {
	print OUTPUT ";" if($symbol_count);
	print OUTPUT "\n     ${_}*";
	$symbol_count++;
    }
    print OUTPUT "\n     *" unless($symbol_count);
    print OUTPUT "\n  };\n";
    print OUTPUT "  local:\n";
    print OUTPUT "  extern \"C++\"\n";
    print OUTPUT "  {\n";
    print OUTPUT "    *\n";
    print OUTPUT "  };\n";
    print OUTPUT "};\n";
} elsif("$EXPORT_FORMAT" eq "symbol_list") {
   foreach (keys %CLASSES) {
	my @symbols = ("${_}::*", "${_}?virtual?table", "${_}?type_info?*", "vtable?for?${_}", 
		       "non-virtual?thunk?to?${_}::*", "typeinfo?for?${_}");
	foreach (@symbols) {
	    print OUTPUT "${_}\n";
	}
    }
    foreach (keys %GLOBALS) {
	print OUTPUT "${_}*\n";
    }
} else {
    print "Uknown format: $EXPORT_FORMAT\n";
}
close(OUTPUT);
exit 0;

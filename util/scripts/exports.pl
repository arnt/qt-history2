#!/usr/bin/perl -w

use strict;

my $EXPORT_OUT = "-";
my $EXPORT_DIRECTORY = 0;
my $EXPORT_SYMBOL = "Q.*_[^ ]*_EXPORT(?:_[^ ]*)?";
while($#ARGV >= 0) {
    if($ARGV[0] eq "-o") {
	shift;
	$EXPORT_OUT = $ARGV[0];
    } else {
	last;
    }
    shift;
}
($#ARGV != 0) && die "$0 [options] directory <export_symbol>\n";
$EXPORT_DIRECTORY = $ARGV[0];
$EXPORT_SYMBOL = $ARGV[1] if($#ARGV == 1);


my %CLASSES=();
my %GLOBALS=();

sub find_classnames {
    my $ret = 0;
    my ($file) = @_;

    my $parsable = "";
    if(open(F, "<$file")) {
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
	    $line =~ s,//.*$,,; #remove c++ comments
	    $parsable .= $line if($line);
	}
	close(F);
    }

    my $last_definition = 0;
    for(my $i = 0; $i < length($parsable); $i++) {
	my $definition = 0;
	my $character = substr($parsable, $i, 1);
	if($character eq "/" && substr($parsable, $i+1, 1) eq "*") { #I parse like this for greedy reasons
	    for($i+=2; $i < length($parsable); $i++) {
		my $end = substr($parsable, $i, 2);
		if($end eq "*/") {
		    $last_definition = $i+2;
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
	    } elsif($definition =~ m/^ *(template<.*> *)?(class|struct) +$EXPORT_SYMBOL +([^ ]+) ?((,|:) *(public|private) *.*)? *\{\}$/) {
		my $symbol = $3;
		$symbol =~ s/[<>+=*!-]/?/g;
		$CLASSES{$symbol} = "$file" unless(!length "$symbol" || defined $CLASSES{$symbol});
	    } elsif($definition =~ /$EXPORT_SYMBOL +([a-zA-Z0-9_\*& ]* +)?[&\*]?([a-zA-Z0-9>=!\*<+_-]*)( *\(.*\))? *(;|\{\})$/) {
		my $symbol = $2;
		$symbol =~ s/[<>+=*!-]/?/g;
		$GLOBALS{$symbol} = "$file" unless(!length "$symbol" || defined $GLOBALS{$symbol});
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
	    } elsif($p =~ /\.(h|cpp)$/) {
		find_classnames("$p");
	    }
	}
	closedir(D);
    }
}
find_files("$EXPORT_DIRECTORY");

#generate output
if("$EXPORT_OUT" eq "-") {
    open(OUTPUT, ">&STDOUT");
} else {
    open(OUTPUT, ">$EXPORT_OUT");
}
print OUTPUT "VERSION\n";
print OUTPUT "{\n";
print OUTPUT "  global:\n";
print OUTPUT "  extern \"C++\"\n";
print OUTPUT "  {";
my $symbol_count = 0;
foreach (keys %CLASSES) {
     my @symbols = ("${_}::*", "${_}?virtual?table", "${_}?type_info?node", "${_}?type_info?function",
		    "vtable?for?${_}", "typeinfo?for?${_}", "non-virtual?thunk?to?${_}::*");
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
close(OUTPUT);

#!/usr/bin/perl

while ( <> ) {
    s/%%(.*)$/~~$1~~/;		# keep until later
    s/%.*//;			# strip comments
    s/\s+/ /g;			# simplify white space
    s/^\s*//;			# strip leading white space
    s/\s*$//;			# strip trailing white space
    s/~~(.*)~~/%%$1/g;		# new line for %% comments
    next if !$_;		# skip empty lines
    if ( !$longline ) {
	$longline = $_;
    }
    else {
	$longline .= " ";
	$longline .= $_;
    }
#    print $_, "\n";
}

print '"', $1, '\n"', "\n" while ( $longline =~ s/(^.{1,74}) // );
print '"', $longline, '\n"', "\n";
exit;

#!/usr/bin/perl

$n = "ark";
$n[0] = 'u';
print $n, "\n";
exit;

while ( <> ) {
    s/[^%]%[^%].*//;		# strip comments
    s/\s+/ /g;			# simplify white space
    s/^\s*//;			# strip leading white space
    s/\s*$//;			# strip trailing white space
    s/(%%[^ ]+)/\n$1\n/g;	# new line for %% comments
    next if !$_;
    if ( !$longline ) {
	$longline = $_;
    }
    else {
	$longline .= ' ';
	$longline .= $_;
    }
#    print $_, "\n";
}

$longline .= "\n";

$maxlen = length $longline;
$i = 76;
while ( $i < $maxlen ) {
    if ( $longline[$i] == 32 ) {
	$longline[i] = "\n"; # eller kanskje: substr($longline,$i,1) = "\n"';
	$i += 76;
    }
    $i--;
}

print $longline;
exit;

#print join( ' ', split($longline,'%') );

#virker ikke
exit;

while ( 1 ) {
    $longline =~ s/(^.{60,} )//;
    if ( $1 ) {
#    if ( $longline =~ s/(^.{60}\s)// ) {
	print $1, "\n";
    }
    else {
	print $longline;
	last;
    }
}

exit;

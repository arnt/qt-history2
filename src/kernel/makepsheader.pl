#!/usr/bin/perl

open(INPUT, 'qpsprinter.ps')
  or die "Can't open qpsprinter.ps";

while(<INPUT>) {
  $line = $_;
  chomp $line;
  $line =~ s/%.*$//;
  $line = $line;
  push(@lines, $line);
#  print "$line\n";
}

$h = join(" ", @lines);
$h =~ s,\t+, ,g;
$h =~ s, +, ,g;

#print $h;

# wordwrap at col 76
@head = split(' ', $h);
$line = shift @head;
while( @head ) {
  $token = shift @head;
  chomp $token;
#  print "\nl=$l, len=$len, token=$token.";
  $newline = $line.' '.$token;
  $newline =~ s, /,/,g;
  $newline =~ s, \{,\{,g;
  $newline =~ s, \},\},g;
  $newline =~ s, \[,\[,g;
  $newline =~ s, \],\],g;
  $newline =~ s,\{ ,\{,g;
  $newline =~ s,\} ,\},g;
  $newline =~ s,\[ ,\[,g;
  $newline =~ s,\] ,\],g;
  if ( length( $newline ) > 76 ) {
#    print "\nline=$line\n";
    $header = $header."\n\"".$line."\\n\"";
    $newline = $token;
  }
  $line = $newline;
}
$header = $header."\n\"".$line."\\n\"";


print $header;

MOVED to www/qpl


#!/usr/bin/perl

open( A, "< QPL-ANNOTATED" ) || die;
open( L, "<LICENSE-2.0" ) || die;

while ( <A> ) {
  if ( m/^\s*(\w+):\s*/ ) {
    $currentTag = $1;
    next;
  }
  if ( $currentTag ) {
    $tag{$currentTag} .= $_;
  }
}

$skip = 0;
while ( <L> ) {
  if ( m/^\s*<\!\-\-\s*(\w+):\s*\-\->\s*/ ) {
    if ( $1 eq "skip" ) {
      $skip = 1;
      next;
    }
    if ( $1 eq "endskip" ) {
      $skip = 0;
      next;
    }
    if ( $tag{$1} ) {
      if ( $1 eq "preface" ) {
	print "<strong>";
      } else {
	print "<p><font color=green>\n";
      }
      print $tag{$1};
      if ( $1 eq "preface" ) {
	print "</strong>";
      } else {
	print "</font>\n";
      }
    } else {
      print "<!-- Cannot find text for the tag \"$1\" -->";
    }
  } else {
    if ( !$skip ) {
      print $_;
    }
  }
}

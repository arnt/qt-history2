#!/usr/bin/perl

open(F,"< ../../include/qglobal.h") || die "Can't open qglobal.h";
while ( <F> ) {
    if ( /(_OS_[A-Za-z0-9_]+)/ ) {
	$os{$1} = 1;
    }
    if ( /(_CC_[A-Za-z0-9_]+)/ ) {
	$cc{$1} = 1;
    }
}
close(F);
print "#include <qglobal.h>\n#include <stdio.h>\n\n";
print "int main( int argc, char **argv )\n{\n";
foreach $f ( sort keys %os ) {
    print "#if defined($f)\n    printf(\"Operating system: $f\\n\");\n#endif\n";
}
foreach $f ( sort keys %cc ) {
    print "#if defined($f)\n    printf(\"Compiler: $f\\n\");\n#endif\n";
}
print "    return 0;\n}\n";

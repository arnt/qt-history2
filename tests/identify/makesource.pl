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
print "#include <qapplication.h>\n#include <stdio.h>\n\n";
print "int main( int argc, char **argv )\n{\n";
print "    QApplication a(argc,argv);\n";
foreach $f ( sort keys %os ) {
    print "#if defined($f)\n    printf(\"Operating system: $f\\n\");\n#endif\n";
}
foreach $f ( sort keys %cc ) {
    print "#if defined($f)\n    printf(\"Compiler: $f\\n\");\n#endif\n";
}
print "#if defined(_WS_WIN_)\n";
print "    printf(\"Windows version: \");\n";
print "    switch ( QApplication::winVersion() ) {\n";
print "        case WV_NT:\n";
print "            printf(\"Windows NT\");\n";
print "            break;\n";
print "        case WV_95:\n";
print "            printf(\"Windows 95\");\n";
print "            break;\n";
print "        case WV_98:\n";
print "            printf(\"Windows 98\");\n";
print "            break;\n";
print "        case WV_32s:\n";
print "            printf(\"Windows 32s\");\n";
print "            break;\n";
print "        default:\n";
print "            printf(\"Unknown\");\n";
print "            break;\n";
print "    }\n";
print "#endif\n";
print "    return 0;\n}\n";

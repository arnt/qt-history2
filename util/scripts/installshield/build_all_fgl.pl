#!/usr/bin/perl
#
# Builds all InstallShield file lists.
#
# Copyright (c) 1998 by Troll Tech AS. All rights reserved.
#

$dir = $ENV{"QTINST_DIR"};
$in = $ENV{"QTINST_IN"};

if ( !($dir && $in) ) {
    print STDERR "Please set the environment variables QTINST_DIR and QTINST_IN\n";
    print STDERR "  QTINST_DIR : InstallShield project directory, e.g. D:\\Installations\\Qt\n";
    print STDERR "  QTINST_IN :  Unpacked Qt distribution, e.g. D:\\qt_inst\\qt\n";
    exit 1;
}

build("Readme",     "");
build("Headers",    "   -t include");
build("Sources",    "-r -t src");
build("Moc",        "   -t bin");
build("Examples",   "-r -t examples");
build("Tutorial",   "-r -t tutorial");
build("HTML",       "-r -t html");
build("ImageIOExt", "-r -t extensions\\imageio");
build("NSPluginExt","-r -t extensions\\nsplugin");
build("OpenGLExt",  "-r -t extensions\\opengl");
$in = $ENV{"QTINST_IN"} . "\\mkfiles\\borland";
build("BorlandMk",  "-r");
$in = $ENV{"QTINST_IN"} . "\\mkfiles\\msvc";
build("MSVCMk",     "-r");
$in = $ENV{"QTINST_IN"};

build_summary();
exit 0;


sub build {
    my($f,$a) = @_;
    push @groups, $f;
    `perl build_fgl.pl -o "$dir\\File Groups\\$f.fgl" $a $in`;
}


sub build_summary {
    my($i);
    local(*F);
    open( F, "> $dir\\File Groups\\Default.fdf") ||
	die "Could not create Default.fdf";

    print F "[Info]\n";
    print F "Type=FileGrp\n";
    print F "Version=1.00.000\n";
    print F "Name=\n\n";

    print F "[FileGroups]\n";
    $i = 0;
    for ( @groups ) {
	print F "group$i=$_\n";
	$i++;
    }
    print F "\n";
    for ( @groups ) {
	print F "[$_]\n";	
	print F "SELFREGISTERING=No\n";
	print F "HTTPLOCATION=\n";
	print F "LANGUAGE=\n";
	print F "OPERATINGSYSTEM=\n";
	print F "FTPLOCATION=\n";
	print F "FILETYPE=No\n";
	print F "INFOTYPE=Standard\n";
	print F "COMMENT=\n";
	print F "COMPRESS=Yes\n";
	print F "COMPRESSDLL=\n";
	print F "POTENTIALLY=No\n";
	print F "MISC=\n\n";
    }
}

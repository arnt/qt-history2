#!
#! This is a custom template for creating a Makefile for the moc.
#!
#${
    StdInit();
    $tsrc = $project{"TOOLSRC"} . " " . $project{"MOCGEN"};
    $tobj = Objects($tsrc);
    $tobj =~ s=\.\.[\\/]tools[\\/]==g;
    $project{"SOURCES"} = $tsrc;
    $project{"OBJECTS"} = $tobj;
    $project{"CLEAN_FILES"} = $project{"TOOLSRC"};
#$}
#$ IncludeTemplate("app.t");

####### Lex/yacc programs and options

LEX	=	flex
YACC	=	byacc -d

####### Lex/yacc files

LEXIN	=	#$ Expand("LEXINPUT");
LEXOUT  =	lex.yy.c
YACCIN	=	#$ Expand("YACCINPUT");
YACCOUT =	y.tab.c
YACCHDR =	y.tab.h
MOCGEN  =	mocgen.cpp

####### Process lex/yacc files

$(LEXOUT): $(LEXIN)
	$(LEX) moc.l

$(MOCGEN): moc.y $(LEXOUT)
	$(YACC) moc.y
	#$ $text = ($is_unix ? "-rm -f " : "-del ") . '$(MOCGEN)'
	#$ $text = ($is_unix ? "mv " : "ren ") . '$(YACCOUT) $(MOCGEN)'; 

####### Use local copy of tools files

#${
    if ( $is_unix ) {
	$cp = "-cp ";
	$td = "../tools/";
    } else {
	$cp = "-copy ";
	$td = "..\\tools\\";
    }
    @s = split(/\s+/,$project{"TOOLSRC"});
    foreach ( @s ) {
	$text && ($text .= "\n");
	$text .= $_ . ": " . $td . $_ . "\n\t" . $cp . $td . $_ . " .\n";
    }
#$}

#!
#! This is a custom template for creating a Makefile for the moc.
#!
#${
    StdInit();
    $tsrc = $project{"TOOLSRC"} . " " . $project{"MOCGEN"};
    $tobj = Objects($tsrc);
    $tobj =~ s=\.\.[\\/]tools[\\/]==g;
    $project{"OBJECTS"} = $tobj;
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
MOCGEN  =	#$ Expand("MOCGEN");

####### Process lex/yacc files

$(LEXOUT): $(LEXIN)
	$(LEX) moc.l

$(MOCGEN): moc.y $(LEXOUT)
	$(YACC) moc.y
	#$ $text = ($is_unix ? "-rm -f " : "-del ") . '$(MOCGEN)'
	#$ $text = ($is_unix ? "-mv " : "-ren ") . '$(YACCOUT) $(MOCGEN)'; 

####### Compile the C++ sources

#$ $text = Objects($project{"MOCGEN"}) . ": " . $project{"MOCGEN"};
	$(CC) -c $(CFLAGS) $(INCPATH) $? -o $@

#${
    if ( $is_unix ) {
	$td = "../tools/";
    } else {
	$td = "..\\tools\\";
    }
    @s = split(/\s+/,$project{"TOOLSRC"});
    foreach ( @s ) {
	$text && ($text .= "\n");
	$text .= Objects($_) . ": " . $td . $_ . "\n\t";
	$text .= '$(CC) -c $(CFLAGS) $(INCPATH) -o $@ $?';
	$text .= "\n";
    }
#$}

#!
#! This is a custom template for creating a Makefile for the moc on Unix.
#!
#############################################################################
# Qt Makefile - Meta Object Compiler (moc)
#############################################################################
#${
    StdInit();
    $tsrc = $project{"TOOLSRC"} . " " . $project{"MOCGEN"};
    $tobj = Objects($tsrc);
    $tobj =~ s=\.\.[\\/]tools[\\/]==g;
    $project{"OBJECTS"} = $tobj;
#$}

####### Directories

INCDIR	=	../tools

####### Compiler

CFLAGS	=	#$ Expand("CFLAGS");
CC	=	#$ Expand("CC");

####### Extra flags for yacc-written code

YACCCFLAGS = -Wno-unused -Wno-parentheses

# $Source: /tmp/cvs/qt/src/moc/Attic/moc-unix.t,v $

####### Lex/yacc programs and options

LEX	=	flex
YACC	=	yacc -d

####### Files

LEXIN	=	#$ Expand("LEXINPUT");
LEXOUT  =	lex.yy.c
YACCIN	=	#$ Expand("YACCINPUT");
YACCOUT =	y.tab.c
YACCHDR =	y.tab.h
MOCGEN  =	#$ Expand("MOCGEN");
OBJECTS =	#$ ExpandList("OBJECTS");
TARGET	=	#$ Expand("TARGET");

####### Implicit rules

.SUFFIXES: .cpp

.cpp.o:
	$(CC) -c $(CFLAGS) -I. -I$(INCDIR) $<

####### Build rules

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET)

depend:
	makedepend -I$(INCDIR) $(LEXIN) $(YACCIN) 2> /dev/null

clean:
	-rm -f $(OBJECTS) $(YACCOUT) $(YACCHDR)
	-rm -f $(TARGET)

####### Process lex/yacc files

$(LEXOUT): $(LEXIN)
	$(LEX) $(LEXIN)

$(MOCGEN): moc.y $(LEXOUT)
	$(YACC) moc.y
	-rm -f $(MOCGEN)
	-mv $(YACCOUT) $(MOCGEN)

####### Compile the C++ sources

#$ $text = Objects($project{"MOCGEN"}) . ": " . $project{"MOCGEN"};
#$ $text = "\t" . '$(CC) -c $(CFLAGS) $(YACCCFLAGS) -I$(INCDIR) ' . $project{"MOCGEN"} . " -o " . Objects($project{"MOCGEN"});

#${
    @s = split(/\s+/,$project{"TOOLSRC"});
    foreach ( @s ) {
	$text && ($text .= "\n");
	$src = "../tools/" . $_;
	$text .= Objects($_) . ": " . $src . "\n\t";
	$text .= '$(CC) -c $(CFLAGS) -I$(INCDIR) ';
	$text .= $src . " -o " . Objects($_) . "\n";
    }
#$}

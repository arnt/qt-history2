
%{

#include <stdio.h>    
#include <qstrlist.h>
#include <malloc.h>
#include <qregexp.h>

#include "project.h"

QMakeProject *g_proj = NULL;


static QStringList operands;
static enum { S_ADD, S_DEL, S_SET } op; 
static QString scope, var;

    
extern "C" int yyparse();
extern "C" int yylex();
extern "C" int yyerror(const char *);
int line_count = 1; 

/* %type <integer> assignment_operator */
%}

%start project

%union
{
    char *string;
    int integer;
}

%token <string> ATOM STRING VARIABLE
%token PLUS EQUALS MINUS COLON EOL
%token COMMENT



%%
project : /* nothing */
        | project COMMENT
        | COMMENT
	| project statement
	| statement
;

statement : assignment
;          

/* assignment logic */
{
    operands.clear();
    scope = "";
    var = "";
}
assignment : assignment_variable assignment_operator assignment_value_list EOL
{
#ifdef QMAKE_DEBUG    
    printf("Project parser: %s (%s) :: %s %d (%s)\n", scope.isEmpty() ? "Empty" : scope.latin1(),
	   g_proj->isActiveConfig(scope) ? "on" : "off", var.latin1(), op, operands.join(" ").latin1());
#endif
    if(g_proj->isActiveConfig(scope))
    {
	QStringList &list = g_proj->variables()[var];
	if(op == S_SET)
	    list.clear();
	for(QStringList::Iterator it = operands.begin(); it != operands.end(); ++it) {
	    switch(op)
	    {
	    case S_SET:
	    case S_ADD:
		if(!list.contains((*it)))
		    list.append((*it));
		break;
	    case S_DEL:
		list.remove((*it));
		break;
	    }
	}
    }
}
;

assignment_variable : ATOM { var = $1; free($1); }
		    | ATOM COLON ATOM { scope = $1; var = $3; free($1); }
;

assignment_operator : EQUALS { op = S_SET; }
                    | PLUS EQUALS { op = S_ADD; }
                    | MINUS EQUALS { op = S_DEL; }
;

assignment_value_list : /* nothing */
                      | assignment_value_list assignment_value
                      | assignment_value
;

assignment_value : STRING {
    QString var = $1; free($1);
    int rep;
    while((rep = var.find(QRegExp("\\$\\$[a-zA-Z0-9_-]*"))) != -1) {
	QString torep = var.mid(rep, var.find(QRegExp("[^\\$a-zA-Z0-9_-]"), rep) - rep);
	QStringList &l = g_proj->variables()[torep.right(torep.length()-2)];
#ifdef QMAKE_DEBUG
	printf("Project parser: (%s) :: %s -> %s\n", var.latin1(), torep.latin1(), l.join(" ").latin1());
#endif	
	if(l.count() > 1) {
	    var.replace(rep, torep.length(), "");
	    operands += l;
	}
	else
	    var.replace(rep, torep.length(), l.first());
    }
    operands.append(var);
}
;



/****************************************************************************
** $Id: //depot/qt/main/src/moc/moc.l#5 $
**
** Lexical analyzer for meta object compiler
**
** Author  : Haavard Nord
** Created : 930417
**
** Copyright (C) 1993,1994 by Haavard Nord.  All rights reserved.
**
** --------------------------------------------------------------------------
** This file contains the lexical analyzer for the meta object compiler (moc)
** if the Quasar application framework.
*****************************************************************************/

%{
#include <qstring.h>


#define input yyinput				// yyinput in C++

#define X if(lexdebug){fprintf(stderr,"LEX (%i) : %s\n",lineNo,yytext);};
#define Y if(lexdebug){fprintf(stderr,"LEX (%i) : %s updates level to %i\n"\
                               ,lineNo,yytext,level);};
#define Z if(lexdebug){fprintf(stderr,"LEX (%i) : skipped the string %s\"\n"\
                               ,lineNo,yytext);};
#define BEGIN_INSIDE


#define  linput() (c = input()) == '\n' ? (lineNo++,c) : c

#include <string.h>
#include <malloc.h>

/* char c,c1; */
int level = 1;    /* Used to keep track of nested curly braces in INSIDE */
%}

%start OUTSIDE CLASS_DEF INSIDE FINISHED

%%
<OUTSIDE>"class" 		{ X;
				  BEGIN CLASS_DEF;
				  return CLASS; }
<INSIDE>"methods:"		{ X;return METHODS; }
<CLASS_DEF>"methods:"		{ X;return METHODS; }
<INSIDE>"signals:"              { X;return SIGNALS; }
<CLASS_DEF>"signals:"           { X;return SIGNALS; }
<INSIDE>"slots:"             	{ X;return SLOTS; }
<CLASS_DEF>"slots:"          	{ X;return SLOTS; }
<INSIDE>"{"                     { level++;Y; }
<INSIDE>"}"                     { level--;Y;if (level == 0) {X;return '}';} }
<INSIDE>"public"		{ X;return PUBLIC; }
<CLASS_DEF>"friend"		{ X;return FRIEND; }
<CLASS_DEF>"typedef"		{ X;return TYPEDEF; }
<CLASS_DEF>"auto"		{ X;return AUTO; }
<CLASS_DEF>"register"		{ X;return REGISTER; }
<CLASS_DEF>"static"		{ X;return STATIC; }
<CLASS_DEF>"extern"		{ X;return EXTERN; }
<CLASS_DEF>"inline"		{ X;return INLINE; }
<CLASS_DEF>"virtual"		{ X;return VIRTUAL; }
<CLASS_DEF>"const"		{ X;return CONST; }
<CLASS_DEF>"volatile"		{ X;return VOLATILE; }
<CLASS_DEF>"char"		{ X;return CHAR; }
<CLASS_DEF>"short"		{ X;return SHORT; }
<CLASS_DEF>"int"		{ X;return INT; }
<CLASS_DEF>"long"		{ X;return LONG; }
<CLASS_DEF>"signed"		{ X;return SIGNED; }
<CLASS_DEF>"unsigned"		{ X;return UNSIGNED; }
<CLASS_DEF>"float"		{ X;return FLOAT; }
<CLASS_DEF>"double"		{ X;return DOUBLE; }
<CLASS_DEF>"void"		{ X;return VOID; }
<CLASS_DEF>"enum"		{ X;return ENUM; }
<CLASS_DEF>"class"		{ X;return CLASS; }
<CLASS_DEF>"struct"		{ X;return STRUCT; }
<CLASS_DEF>"union"		{ X;return UNION; }
<CLASS_DEF>"asm"		{ X;return ASM; }
<CLASS_DEF>"private"		{ X;return PRIVATE; }
<CLASS_DEF>"protected"		{ X;return PROTECTED; }
<CLASS_DEF>"public"		{ X;return PUBLIC; }
<CLASS_DEF>"operator"		{ X;return OPERATOR; }
<CLASS_DEF>"::"			{ X;return DBL_COLON; }
<CLASS_DEF>"..."                { X;return TRIPLE_DOT; }

<CLASS_DEF>[_a-zA-Z][_a-zA-Z0-9]* {
			  X;
                          yylval.string = strdup(yytext);
			  return IDENTIFIER;
                        }

<INSIDE>\"[^"]*      	{			/* discard strings */
                          if (yytext[yyleng - 1] == '\\') {
                            yymore();
       			  }
                          else {
                            Z;
                            input();   		/* discard the '"' */
                          }
                        }

<CLASS_DEF>\"[^"]*      { if (yytext[yyleng - 1] == '\\') {
                              yymore();
			  }
                          else {
                              X;
                              yylval.string = strdup(yytext);
                              input();   	/* discard the '"' */
                              return STRING;
                         }
                        }

<CLASS_DEF>'.'          { X;
                          yylval.char_val = yytext[1];
                          return CHAR_VAL;
                        }

<CLASS_DEF>'\\a'       { X;
                         yylval.char_val = '\a';
                         return CHAR_VAL;
                       }

<CLASS_DEF>'\\b'       { X;
                         yylval.char_val = '\b';
                         return CHAR_VAL;
                       }

<CLASS_DEF>'\\f'       { X;
                         yylval.char_val = '\f';
                         return CHAR_VAL;
                       }

<CLASS_DEF>'\\n'       { X;
                         yylval.char_val = '\n';
                         return CHAR_VAL;
                       }

<CLASS_DEF>'\\r'       { X;
                         yylval.char_val = '\r';
                         return CHAR_VAL;
                       }

<CLASS_DEF>'\\t'       { X;
                         yylval.char_val = '\t';
                         return CHAR_VAL;
                       }

<CLASS_DEF>'\\v'       { X;
                         yylval.char_val = '\v';
                         return CHAR_VAL;
                       }

<CLASS_DEF>'\\\\'      { X;
                         yylval.char_val = '\\';
                         return CHAR_VAL;
                       }

<CLASS_DEF>'\\?'       { X;
                         yylval.char_val = '\?';
                         return CHAR_VAL;
                       }

<CLASS_DEF>'\\''       { X;
                         yylval.char_val = '\'';
                         return CHAR_VAL;
                       }

<CLASS_DEF>'\\\"'      { X;
                         yylval.char_val = '\"';
                         return CHAR_VAL;
                       }

<CLASS_DEF>'\\0'       { X;
                         yylval.char_val = '\0';
                         return CHAR_VAL;
                       }

<CLASS_DEF>'\\v'       { X;
                         yylval.char_val = '\v';
                         return CHAR_VAL;
                       }

<CLASS_DEF>'\\[0-7]+'  { X;
                         yylval.char_val =
			   (char)strtol( &yytext[1], 0, 8 );
                         return CHAR_VAL;
                       }

<CLASS_DEF>'\\x[0-9a-fA-F]+' { X;
                         yylval.char_val =
			   (char)strtol( &yytext[2], 0, 16 );
                         return CHAR_VAL;
                       }

<CLASS_DEF>'\\.'       { X;
                         yylval.char_val = ' ';
                         return CHAR_VAL;
                       }

<CLASS_DEF>[0-9]+       { X;
                          yylval.int_val = atoi(yytext);
                          return INT_VAL;
                        }

<CLASS_DEF>[0-9]+\.[0-9]* { X;
                          yylval.double_val = atof(yytext);
                          return DOUBLE_VAL;
                        }

<CLASS_DEF>\.[0-9]+     { X;
                          yylval.double_val = atof(yytext);
                          return DOUBLE_VAL;
                        }


^#define.*\\$           { /* skip multi-line macro-definitions */
			  char c, c1;
                          input();   /* Discard the '\n'. */
                          do {
                              c1=' ';
                              while((c = linput()) != '\n' && c > 0) c1=c;
                              if (c <= 0) break;
                          } while(c1=='\\');
                          unput(c);  /* put back the '\n' or the EOF */
                        }

^#.*                    { /* preprocessor commands are skipped */}
"//"[^\n]*              { /* C++ comment */ }
"/*"                    { /* C   comment */
			  char c;
                          do {
                              while((c = linput()) != '*' && c > 0);
                              if (c < 0) break;
                          } while(((c = linput())) != '/' && c > 0);
                          if (c <= 0)
                              unput(c);
                        }

[ \t]+                  ;
<INSIDE>.               ;
<OUTSIDE>.              ;
<FINISHED>.             ;
<CLASS_DEF>.  		{
			  X;
			  return yytext[0];
			}
\n 			{
			  lineNo++;
			}

/****************************************************************************
** $Id: //depot/qt/main/src/moc/moc.l#1 $
**
** Lexical analyzer for ODE compiler
**
** Author  : Haavard Nord
** Created : 930417
**
** Copyright (C) 1993 by Haavard Nord. All rights reserved.
**
*****************************************************************************/

%{
#include <xstring.h>


#define input lex_input

#define X if(lexdebug){printf("LEX line number %i : %s\n",lineNo,yytext);};
#define Y if(lexdebug){printf("LEX line number %i : %s updates level to %i\n"\
                              ,lineNo,yytext,level);};
#define Z if(lexdebug){printf("LEX line number %i : skipped the string %s\"\n"\
                              ,lineNo,yytext);};
#define BEGIN_INSIDE 


#define  linput() (c = input()) == '\n' ? (lineNo++,c) : c

#include <string.h>
#include <malloc.h>

char c,c1;
int level = 1;    /* Used to keep track of nested curly braces in INSIDE */
%}

%START OUTSIDE CLASS_DEF INSIDE FINISHED

%%
<OUTSIDE>"class" 		{X;
				 BEGIN CLASS_DEF;
				 return CLASS;}

<INSIDE>"signals:"              {X;return SIGNALS;}
<CLASS_DEF>"signals:"           {X;return SIGNALS;}
<INSIDE>"slots:"             	{X;return SLOTS;}
<CLASS_DEF>"slots:"          	{X;return SLOTS;}
<INSIDE>"{"                     {level++;Y;}
<INSIDE>"}"                     {level--;Y;if (level == 0) {X;return '}';}}
<INSIDE>"public"		{X;return PUBLIC;}
<CLASS_DEF>"public"             {X;return PUBLIC;}
<CLASS_DEF>"private"            {X;return PRIVATE;}
<CLASS_DEF>"protected"		{X;return PROTECTED;}
<CLASS_DEF>"virtual"		{X;return VIRTUAL;}
<CLASS_DEF>"friend"		{X;return FRIEND;}
<CLASS_DEF>"typedef"		{X;return TYPEDEF;}
<CLASS_DEF>"auto"		{X;return AUTO;}
<CLASS_DEF>"register"		{X;return REGISTER;}
<CLASS_DEF>"static"		{X;return STATIC;}
<CLASS_DEF>"extern"		{X;return EXTERN;}
<CLASS_DEF>"inline"		{X;return INLINE;}
<CLASS_DEF>"const"		{X;return CONST;}
<CLASS_DEF>"volatile"		{X;return VOLATILE;}
<CLASS_DEF>"short"		{X;return SHORT;}
<CLASS_DEF>"char"		{X;return CHAR;}
<CLASS_DEF>"int"		{X;return INT;}
<CLASS_DEF>"bool"               {X;return BOOL;}
<CLASS_DEF>"long"		{X;return LONG;}
<CLASS_DEF>"signed"		{X;return SIGNED;}
<CLASS_DEF>"unsigned"		{X;return UNSIGNED;}
<CLASS_DEF>"float"		{X;return FLOAT;}
<CLASS_DEF>"double"		{X;return DOUBLE;}
<CLASS_DEF>"void"		{X;return VOID;}
<CLASS_DEF>"enum"		{X;return ENUM;}
<CLASS_DEF>"operator"		{X;return OPERATOR;}
<CLASS_DEF>"::"			{X;return DBL_COLON;}
<CLASS_DEF>"..."                {X;return TRIPLE_DOT;}


<CLASS_DEF>"TRUE"               {X; /*yylval.boolean = TRUE ;*/return BOOLEAN;}
<CLASS_DEF>"FALSE"              {X; /*yylval.boolean = FALSE;*/return BOOLEAN;}



<CLASS_DEF>[_a-zA-Z][_a-zA-Z0-9]* {    X;
                                       yylval.string = strdup(yytext);
				       return IDENTIFIER;
                                  }

<INSIDE>\"[^"]*      {    /* Discard strings */
                          if (yytext[yyleng - 1] == '\\')
                              yymore();
                          else {
                              Z;
                              input();   /* discard the '"' */
                         }
                        }

<CLASS_DEF>\"[^"]*      { if (yytext[yyleng - 1] == '\\')
                              yymore();
                          else {
                              X;
                              yylval.string = strdup(yytext);
                              input();   /* discard the '"' */
                              return STRING;
                         }
                        }

<CLASS_DEF>'.'          { X;
                          yylval.character = yytext[1];
                          return CHARACTER;
                        }

<CLASS_DEF>'\\a'       { X;
                         yylval.character = '\a';
                         return CHARACTER;
                       }

<CLASS_DEF>'\\b'       { X;
                         yylval.character = '\b';
                         return CHARACTER;
                       }

<CLASS_DEF>'\\f'       { X;
                         yylval.character = '\f';
                         return CHARACTER;
                       }

<CLASS_DEF>'\\n'       { X;
                         yylval.character = '\n';
                         return CHARACTER;
                       }

<CLASS_DEF>'\\r'       { X;
                         yylval.character = '\r';
                         return CHARACTER;
                       }

<CLASS_DEF>'\\t'       { X;
                         yylval.character = '\t';
                         return CHARACTER;
                       }

<CLASS_DEF>'\\v'       { X;
                         yylval.character = '\v';
                         return CHARACTER;
                       }

<CLASS_DEF>'\\\\'      { X;
                         yylval.character = '\\';
                         return CHARACTER;
                       }

<CLASS_DEF>'\\?'       { X;
                         yylval.character = '\?';
                         return CHARACTER;
                       }

<CLASS_DEF>'\\''       { X;
                         yylval.character = '\'';
                         return CHARACTER;
                       }

<CLASS_DEF>'\\\"'      { X;
                         yylval.character = '\"';
                         return CHARACTER;
                       }

<CLASS_DEF>'\\0'       { X;
                         yylval.character = '\0';
                         return CHARACTER;
                       }

<CLASS_DEF>'\\v'       { X;
                         yylval.character = '\v';
                         return CHARACTER;
                       }

<CLASS_DEF>'\\[0-9][0-9][0-9]' { X;
                                 yylval.character  = yytext[2] * 100 +
                                                     yytext[3] * 10  +
                                                     yytext[4];
                                 return CHARACTER;
                       }

<CLASS_DEF>'\\x[0-9][0-9]' { X;
                             yylval.character  = yytext[3] * 16 +
                                                 yytext[4];
                             return CHARACTER;
                       }

<CLASS_DEF>'\\.'       { X;
                         yylval.character = ' ';
                         return CHARACTER;
                       }

<CLASS_DEF>[0-9]+       { X;
                          yylval.integer = atoi(yytext);
                          return INTEGER;
                        }

<CLASS_DEF>[0-9]+\.[0-9]* { X;
                            yylval.float_num = atof(yytext);
                            return FLOAT_NUM;
                          }

<CLASS_DEF>\.[0-9]+     { X;
                          yylval.float_num = atof(yytext);
                          return FLOAT_NUM;
                        }


^#define.*\\$           { /*Skip multi-line macro-definitions.*/
                          input();   /* Discard the '\n'. */
                          do {
                              c1=' ';
                              while((c = linput()) != '\n' && c > 0) c1=c;
                              if (c <= 0) break;
                          } while(c1=='\\');
                          unput(c);  /* put back the '\n' or the EOF */
                        }

^#.*                    { /* Preprocessor commands are skipped */}
"//"[^\n]*              { /* C++ comment */ }
"/*"                    { /* C   comment */
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

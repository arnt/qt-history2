%{
#include <stdio.h>
#include <string.h>
#include "y.tab.h"
extern int line_count;

%}

%x literal
legal_atom_char [a-zA-Z0-9_\-]

%%

= { BEGIN(literal); return EQUALS; }
- { return MINUS; }
\+ { return PLUS; }
: { return COLON; }
#.*\n { line_count++; return COMMENT; } 
\\\n { line_count++; } /* continuation line */
^\n  { line_count++; } /* blank line */
; { return EOL; }
(;\n|\n) { line_count++; return EOL; }
[ \t]* /* nothing */
{legal_atom_char}* { yylval.string=strdup(yytext); return ATOM; }

<literal>#.*$ /* nothing */
<literal>\\\n { line_count++; } /* continuation line */
<literal>(;\n|\n) { line_count++; BEGIN(INITIAL); return EOL; }
<literal>\".*[^\\]\" { yylval.string=strdup(yytext+1); yylval.string[strlen(yylval.string)-1] = '\0'; return STRING; }
<literal>[ \t]* /* nothing */
<literal>[^ \t\n]* { yylval.string=strdup(yytext); return STRING; }


%%


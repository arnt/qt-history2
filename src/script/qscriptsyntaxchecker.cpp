/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qscriptsyntaxchecker_p.h"
#include "qscriptlexer_p.h"

namespace QScript {


SyntaxChecker::SyntaxChecker():
    tos(0),
    stack_size(0),
    state_stack(0)
{
}

SyntaxChecker::~SyntaxChecker()
{
    if (stack_size) {
        qFree(state_stack);
    }
}

bool SyntaxChecker::automatic(QScript::Lexer *lexer, int token) const
{
    return token == T_RBRACE || token == 0 || lexer->prevTerminator();
}

bool SyntaxChecker::parse(const QString &code)
{
  const int INITIAL_STATE = 0;
  QScript::Lexer lexer (/*engine=*/ 0);
  lexer.setCode(code); // ### set the lineno

  int yytoken = -1;
  int saved_yytoken = -1;

  reallocateStack();

  tos = 0;
  state_stack[++tos] = INITIAL_STATE;

  while (true)
    {
      if (yytoken == -1 && - TERMINAL_COUNT != action_index [state_stack [tos]])
        {
          if (saved_yytoken == -1)
            yytoken = lexer.lex();
          else
            {
              yytoken = saved_yytoken;
              saved_yytoken = -1;
            }
        }

      int act = t_action (state_stack [tos], yytoken);

      if (act == ACCEPT_STATE)
        return true;

      else if (act > 0)
        {
          if (++tos == stack_size)
            reallocateStack();

          state_stack [tos] = act;
          yytoken = -1;
        }

      else if (act < 0)
        {
          int r = - act - 1;

          tos -= rhs [r];
          act = state_stack [tos++];

          state_stack [tos] = nt_action (act, lhs [r] - TERMINAL_COUNT);
        }

      else
        {
          if (saved_yytoken == -1 && automatic (&lexer, yytoken) && t_action (state_stack [tos], T_AUTOMATIC_SEMICOLON) > 0)
            {
              saved_yytoken = yytoken;
              yytoken = T_SEMICOLON;
              continue;
            }

          break;
        }
    }

  return yytoken != 0;
}

} // namespace QScript


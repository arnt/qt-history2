
#line 92 "lalr.g"

#include "recognizer.h"
#include <cstdlib>
#include <cstring>
#include <cctype>

Recognizer::Recognizer (Grammar *grammar, FILE *fp, bool no_lines):
  tos(0),
  stack_size(0),
  state_stack(0),
  ch(' '),
  _M_file(fp),
  _M_line(0),
  _M_action_line(0),
  _M_grammar(grammar),
  _M_no_lines(no_lines)
{
  if (! _M_file)
    _M_file = stdin;
}

Recognizer::~Recognizer()
{
  if (stack_size) {
      ::free(state_stack);
  }
}

inline void Recognizer::reallocateStack()
{
    if (! stack_size)
        stack_size = 128;
    else
        stack_size <<= 1;

    sym_stack.resize (stack_size);
    state_stack = reinterpret_cast<int*> (::realloc(state_stack, stack_size * sizeof(int)));
}

int Recognizer::nextToken()
{
  QString text;

Lagain:
  while (std::isspace (ch))
    inp ();

  int token = ch;

  if (token == EOF)
    return (token = 0);

  else if (token == '"')
    {
      inp(); // skip "
      text.clear ();
      while (ch && ch != '"')
        {
          if (ch == '\\')
            {
              text += ch;
              inp();
            }
          text += ch;
          inp ();
        }

      if (ch == '"')
        inp ();
      else
        fprintf (stderr, "*** Warning. Expected ``\"''\n");

      _M_current_value = text;
      return (token = STRING_LITERAL);
    }

  else if (std::isalnum (token) || token == '_')
    {
      text.clear ();
      do { text += ch; inp (); }
      while (std::isalnum (ch) || ch == '_' || ch == '.');
      _M_current_value = text;
      return (token = ID);
    }

  else if (token == '%')
    {
      text.clear ();

      do { inp (); }
      while (std::isspace (ch));

      do { text += ch; inp (); }
      while (std::isalnum (ch) || ch == '_' || ch == '-');

      if (text == QLatin1String("token_prefix"))
        return (token = TOKEN_PREFIX);
      else if (text == QLatin1String("merged_output"))
        return (token = MERGED_OUTPUT);
      else if (text == QLatin1String("token"))
        return (token = TOKEN);
      else if (text == QLatin1String("start"))
        return (token = START);
      else if (text == QLatin1String("parser"))
        return (token = PARSER);
      else if (text == QLatin1String("decl"))
        return (token = DECL_FILE);
      else if (text == QLatin1String("impl"))
        return (token = IMPL_FILE);
      else if (text == QLatin1String("expect"))
        return (token = EXPECT);
      else if (text == QLatin1String("expect-rr"))
        return (token = EXPECT_RR);
      else if (text == QLatin1String("left"))
        return (token = LEFT);
      else if (text == QLatin1String("right"))
        return (token = RIGHT);
      else if (text == QLatin1String("nonassoc"))
        return (token = NONASSOC);
      else if (text == QLatin1String("prec"))
        return (token = PREC);
      else
        {
          fprintf (stderr, "*** Error. Unknown keyword\n");
          exit (1);
        }
    }

  inp ();

  if (token == '-' && ch == '-')
    {
      do { inp (); }
      while (ch != EOF && ch != '\n');
      goto Lagain;
    }

  else if (token == ':' && ch == ':')
    {
      inp ();
      if (ch != '=')
        return (token = ERROR);
      inp ();
      return (token = COLON);
    }

  else if (token == '/' && ch == ':')
    {
      _M_action_line = _M_line;

      text.clear ();
      if (! _M_no_lines)
        text += QLatin1String ("\n#line ") + QString::number (_M_action_line + 1) + " \"" + _M_input_file + "\"\n";
      inp (); // skip ':'

      while (ch != EOF)
        {
          token = ch;
          inp ();

          if (token == ':' && ch == '/')
            break;

          text += QLatin1Char (token);
        }

      if (ch != '/')
        return (token = ERROR);

      inp ();

      _M_current_value = text;
      return (token = DECL);
    }

  else if (token == '/' && ch == '.')
    {
      _M_action_line = _M_line;

      text.clear ();
      if (! _M_no_lines)
        text += QLatin1String ("\n#line ") + QString::number (_M_action_line + 1) + " \"" + _M_input_file + "\"\n";

      inp (); // skip ':'

      while (ch != EOF)
        {
          token = ch;
          inp ();

          if (token == '.' && ch == '/')
            break;

          text += QLatin1Char (token);
        }

      if (ch != '/')
        return (token = ERROR);
      inp ();

      _M_current_value = text;
       return (token = IMPL);
    }

  switch (token) {
    case ':':
      return (token = COLON);

    case ';':
      return (token = SEMICOLON);

    case '|':
      return (token = OR);

    default:
      break;
  }

  return token;
}

bool Recognizer::parse (const QString &input_file)
{
  const int INITIAL_STATE = 0;

  int yytoken = -1;
  reallocateStack();

  _M_input_file = QLatin1String ("<stdin>");
  if (! input_file.isEmpty ())
    _M_input_file = input_file;

  _M_current_rule = _M_grammar->rules.end ();
  _M_decls.clear ();
  _M_impls.clear ();

  tos = 0;
  state_stack[++tos] = INITIAL_STATE;

  while (true)
    {
      if (yytoken == -1 && - TERMINAL_COUNT != action_index [state_stack [tos]])
        yytoken = nextToken();

      int act = t_action (state_stack [tos], yytoken);

      if (act == ACCEPT_STATE) {
        return true;
      }

      else if (act > 0)
        {
          if (++tos == stack_size)
            reallocateStack();

          sym_stack [tos] = _M_current_value;
          state_stack [tos] = act;
          yytoken = -1;
        }

      else if (act < 0)
        {
          int r = - act - 1;

          tos -= rhs [r];
          act = state_stack [tos++];

          switch (r) {

#line 369 "lalr.g"

case 3: {
  Name name = _M_grammar->intern (sym(2));
  _M_grammar->start = name;
  _M_grammar->non_terminals.insert (name);
} break;

#line 381 "lalr.g"

case 5: {
  _M_grammar->table_name = sym(2);
} break;

#line 388 "lalr.g"

case 6: {
  _M_grammar->merged_output = sym(2);
} break;

#line 395 "lalr.g"

case 7: {
   _M_grammar->decl_file_name = sym(2);
} break;

#line 403 "lalr.g"

case 8: {
   _M_grammar->impl_file_name = sym(2);
} break;

#line 410 "lalr.g"

case 9: {
   _M_grammar->expected_shift_reduce = sym(2).toInt();
} break;

#line 417 "lalr.g"

case 10: {
   _M_grammar->expected_reduce_reduce = sym(2).toInt();
} break;

#line 425 "lalr.g"

case 11: {
  _M_grammar->token_prefix = sym(2);
} break;

#line 443 "lalr.g"
case 17:
#line 446 "lalr.g"
case 18: {
  Name name = _M_grammar->intern (sym(1));
  _M_grammar->terminals.insert (name);
  _M_grammar->spells.insert (name, sym(2));
} break;

#line 454 "lalr.g"

case 19: {
  _M_grammar->current_assoc = Grammar::Left;
  ++_M_grammar->current_prec;
} break;

#line 462 "lalr.g"

case 20: {
  _M_grammar->current_assoc = Grammar::Right;
  ++_M_grammar->current_prec;
} break;

#line 470 "lalr.g"

case 21: {
  _M_grammar->current_assoc = Grammar::NonAssoc;
  ++_M_grammar->current_prec;
} break;

#line 483 "lalr.g"

case 25: {
  Name name = _M_grammar->intern (sym(1));
  _M_grammar->terminals.insert (name);

  Grammar::TokenInfo info;
  info.prec = _M_grammar->current_prec;
  info.assoc = _M_grammar->current_assoc;
  _M_grammar->token_info.insert (name, info);
} break;

#line 497 "lalr.g"

case 26: {
  _M_decls += expand (sym(1));
} break;

#line 505 "lalr.g"

case 27: {
  _M_impls += expand (sym(1));
} break;

#line 522 "lalr.g"

case 34: {
  _M_current_rule = _M_grammar->rules.insert (_M_grammar->rules.end (), Rule ());
  _M_current_rule->lhs = _M_grammar->intern (sym(1));
  _M_grammar->declared_lhs.insert (_M_current_rule->lhs);

  if (_M_grammar->terminals.find (_M_current_rule->lhs) != _M_grammar->terminals.end ())
    {
      fprintf (stderr, "*** Fatal. Invalid non terminal `%s'\n", qPrintable(*_M_current_rule->lhs));
      exit (1);
    }

  _M_grammar->non_terminals.insert (_M_current_rule->lhs);
} break;

#line 545 "lalr.g"

case 38: {
  Name lhs = _M_current_rule->lhs;
  _M_current_rule = _M_grammar->rules.insert (_M_grammar->rules.end (), Rule ());
  _M_current_rule->lhs = lhs;
  _M_grammar->declared_lhs.insert (_M_current_rule->lhs);

  if (_M_grammar->terminals.find (_M_current_rule->lhs) != _M_grammar->terminals.end ())
    {
      fprintf (stderr, "*** Fatal. Invalid non terminal `%s'\n", qPrintable(*_M_current_rule->lhs));
      exit (1);
    }

  _M_grammar->non_terminals.insert (_M_current_rule->lhs);
} break;

#line 563 "lalr.g"

case 39: {
  _M_current_rule->prec = _M_grammar->names.end ();

  for (NameList::iterator it = _M_current_rule->rhs.begin (); it != _M_current_rule->rhs.end (); ++it)
    {
      if (! _M_grammar->isTerminal (*it))
        continue;

      _M_current_rule->prec = *it;
    }
} break;

#line 578 "lalr.g"

case 40: {
  Name tok = _M_grammar->intern (sym(2));
  if (! _M_grammar->isTerminal (tok))
    {
      fprintf (stderr, "*** Warning. `%s' is not a terminal symbol\n", qPrintable (*tok));
      _M_current_rule->prec = _M_grammar->names.end ();
    }
  else
    _M_current_rule->prec = tok;
} break;

#line 594 "lalr.g"

case 42: {
  Name name = _M_grammar->intern (sym(2));

  if (_M_grammar->terminals.find (name) == _M_grammar->terminals.end ())
    _M_grammar->non_terminals.insert (name);

  _M_current_rule->rhs.push_back (name);
} break;

#line 607 "lalr.g"

case 43: {
  sym(1) = QString();
} break;

#line 617 "lalr.g"

          } // switch

          state_stack [tos] = nt_action (act, lhs [r] - TERMINAL_COUNT);
        }

      else
        {
          break;
        }
    }

    fprintf (stderr, "*** Syntax Error at line %d\n", _M_line);
    return false;
}


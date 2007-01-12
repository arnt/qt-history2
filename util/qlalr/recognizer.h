
#line 32 "lalr.g"

#include <QtDebug>
#include <QString>
#include <cstdio>
#include "grammar_p.h"
#include "lalr.h"

class Recognizer: protected grammar
{
public:
  Recognizer (Grammar *grammar, FILE *fp, bool no_lines);
  ~Recognizer();

  bool parse (const QString &input_file = QString ());

  inline QString decls () const { return _M_decls; }
  inline QString impls () const { return _M_impls; }

protected:
  inline void reallocateStack ();

  inline QString &sym (int index)
  { return sym_stack [tos + index - 1]; }

protected: // scanner
  int nextToken();

  inline void inp ()
  {
    ch = ::fgetc (_M_file);

    if (ch == '\n')
      ++_M_line;
  }

  QString expand (const QString &text) const;

protected:
  // recognizer
  int tos;
  int stack_size;
  QVector<QString> sym_stack;
  int *state_stack;

  // scanner
  int ch;
  FILE *_M_file;
  int _M_line;
  int _M_action_line;
  Grammar *_M_grammar;
  RulePointer _M_current_rule;
  QString _M_input_file;

  QString _M_decls;
  QString _M_impls;
  QString _M_current_value;
  bool _M_no_lines;
};

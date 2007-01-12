
#define _CRT_SECURE_NO_DEPRECATE

#include <QtCore/QFile>
#include <QtCore/QtDebug>

#include <cstdlib>

#include "lalr.h"
#include "dotgraph.h"
#include "parsetable.h"
#include "cppgenerator.h"
#include "recognizer.h"

#define QLALR_NO_DEBUG_TABLE
#define QLALR_NO_DEBUG_DOT

static void help_me ()
{
  qerr << "Usage: qlalr [options] [input file name]" << endl
       << endl
       << "  -h, --help\t\tdisplay this help and exit" << endl
       << "  -v, --verbose\t\tverbose output" << endl
       << "  --no-debug\t\tno debug information" << endl
       << "  --no-lines\t\tno #line directives" << endl
       << "  --dot\t\t\tno generate the graph" << endl
       << endl;
  exit (0);
}

int main (int, char *argv[])
{
  bool generate_dot = false;
  bool generate_report = false;
  bool no_lines = false;
  bool debug_info = true;
  const char *file_name = 0;
  FILE *input_file = 0;

  while (const char *arg = *++argv)
    {
      if (! qstrcmp (arg, "--help"))
        help_me ();

      else if (! qstrcmp (arg, "-v") || ! qstrcmp (arg, "--verbose"))
        generate_report = true;

      else if (! qstrcmp (arg, "--dot"))
        generate_dot = true;

      else if (! qstrcmp (arg, "--no-lines"))
        no_lines = true;

      else if (! qstrcmp (arg, "--no-debug"))
        debug_info = false;

      else if (! input_file)
        {
          input_file = fopen (arg, "r");


          if (input_file)
            file_name = arg;
          else
            {
              input_file = stdin;
              qerr << "*** Warning. Reading from standard input" << endl;
            }
        }

      else
        qerr << "*** Warning. Ignore argument `" << arg << "'" << endl;
    }

  if (! input_file)
    {
      input_file = stdin;
      qerr << "*** Warning. Reading from standard input" << endl;
    }

  Grammar grammar;
  Recognizer p (&grammar, input_file, no_lines);

  if (! p.parse (file_name))
    exit (EXIT_FAILURE);

  if (input_file && input_file != stdin)
    fclose (input_file);

  if (grammar.rules.isEmpty ())
    {
      qerr << "*** Fatal. No rules!" << endl;
      exit (EXIT_FAILURE);
    }

  else if (grammar.start == grammar.names.end ())
    {
      qerr << "*** Fatal. No start symbol!" << endl;
      exit (EXIT_FAILURE);
    }

  grammar.buildExtendedGrammar ();
  grammar.buildRuleMap ();

  Automaton aut (&grammar);
  aut.build ();

  CppGenerator gen (p, grammar, aut, generate_report);
  gen.setDebugInfo (debug_info);
  gen ();

  if (generate_dot)
    {
      DotGraph genDotFile (qout);
      genDotFile (&aut);
    }

  else if (generate_report)
    {
      ParseTable genParseTable (qout);
      genParseTable(&aut);
    }

  return EXIT_SUCCESS;
}

QString Recognizer::expand (const QString &text) const
{
  QString code = text;

  if (_M_grammar->start != _M_grammar->names.end ())
    {
      code = code.replace (QLatin1String("$start_id"), QString::number (std::distance (_M_grammar->names.begin (), _M_grammar->start)));
      code = code.replace (QLatin1String("$start"), *_M_grammar->start);
    }

  code = code.replace (QLatin1String("$header"), _M_grammar->table_name.toLower () + QLatin1String("_p.h"));

  code = code.replace (QLatin1String("$table"), _M_grammar->table_name);
  code = code.replace (QLatin1String("$parser"), _M_grammar->table_name);

  if (_M_current_rule != _M_grammar->rules.end ())
    {
      code = code.replace (QLatin1String("$rule_number"), QString::number (std::distance (_M_grammar->rules.begin (), _M_current_rule)));
      code = code.replace (QLatin1String("$rule"), *_M_current_rule->lhs);
    }

  return code;
}

// kate: indent-width 2;

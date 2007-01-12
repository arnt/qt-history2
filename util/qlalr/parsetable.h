#ifndef PARSETABLE_H
#define PARSETABLE_H

class QTextStream;
class Automaton;

class ParseTable
{
public:
  ParseTable (QTextStream &out);

  void operator () (Automaton *a);

private:
  QTextStream &out;
};

#endif // PARSETABLE_H

// kate: indent-width 2;

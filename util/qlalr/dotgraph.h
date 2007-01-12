#ifndef DOTGRAPH_H
#define DOTGRAPH_H

class QTextStream;
class Automaton;

class DotGraph
{
public:
  DotGraph (QTextStream &out);

  void operator () (Automaton *a);

private:
  QTextStream &out;
};

#endif // DOTGRAPH_H

// kate: indent-width 2;

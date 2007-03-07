/****************************************************************************
**
** Copyright (C) 2007-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the QLALR project on Trolltech Labs.
**
** $TROLLTECH_GPL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

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

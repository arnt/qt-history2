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

#ifndef COMPRESS_H
#define COMPRESS_H

#include <QtCore/QVector>

class Compress
{
public:
  Compress ();

  void operator () (int *table, int row_count, int column_count);

public:
  QVector<int> index;
  QVector<int> info;
  QVector<int> check;
};

#endif // COMPRESS_H

// kate: indent-width 2;

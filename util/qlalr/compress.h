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

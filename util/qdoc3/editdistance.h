/*
  editdistance.h
*/

#ifndef EDITDISTANCE_H
#define EDITDISTANCE_H

#include <qstring.h>

#include "set.h"

int editDistance( const QString& s, const QString& t );
QString nearestName( const QString& actual, const Set<QString>& candidates );

#endif

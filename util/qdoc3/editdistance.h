/*
  editdistance.h
*/

#ifndef EDITDISTANCE_H
#define EDITDISTANCE_H

#include <QSet>
#include <QString>

int editDistance( const QString& s, const QString& t );
QString nearestName( const QString& actual, const QSet<QString>& candidates );

#endif

/*
  codeprocessor.h
*/

#ifndef CODEPROCESSOR_H
#define CODEPROCESSOR_H

#include <qmap.h>
#include <qstring.h>

#include "stringset.h"

class Resolver;

typedef QMap<int, StringSet> OccurrenceMap;

OccurrenceMap occurrenceMap( const QString& code, const Resolver *res,
			     const QString& dirPath = QString::null );
QString processCodeHtml( const QString& code, const Resolver *res = 0,
			 const QString& dirPath = QString::null,
			 bool localLinks = FALSE );

#endif

/*
  codeprocessor.h
*/

#ifndef CODEPROCESSOR_H
#define CODEPROCESSOR_H

#include <qstring.h>

class Resolver;

QString processCodeHtml( const QString& code, const Resolver *res = 0,
			 bool localLinks = FALSE,
			 const QString& dirPath = QString::null );

#endif

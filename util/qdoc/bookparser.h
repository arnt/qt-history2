/*
  bookparser.h
*/

#ifndef BOOKPARSER_H
#define BOOKPARSER_H

class QString;

class Resolver;

enum { Html = 0x1, Sgml = 0x2 };

void parseBookFile( const QString& filePath, int fmt,
		    const Resolver *resolver );

#endif

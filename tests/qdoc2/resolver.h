/*
  resolver.h
*/

#ifndef RESOLVER_H
#define RESOLVER_H

#include <qstring.h>

class Location;

class Resolver
{
public:
    virtual ~Resolver() { }

    virtual QString resolve( const QString& name ) const;
    virtual QString resolvefn( const QString& name ) const;
    virtual void compare( const Location& loc, const QString& link,
			  const QString& html ) const;

    QString href( const QString& name,
		  const QString& text = QString::null ) const;
};

#endif

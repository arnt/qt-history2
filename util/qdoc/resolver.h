/*
  resolver.h
*/

#ifndef RESOLVER_H
#define RESOLVER_H

#include <qstring.h>

class Location;

/*
  The Resolver class is a base class for resolvers of identifiers.

  The function resolve() returns a link to the documentation of an
  identifier. For example, resolving 'QRegExp::search()' in Qt should
  give 'qregexp.html#search'. Function href() is provided for
  convenience.  It returns a string of the style
  '<a href=qregexp.html#search>QRegExp::search</a>()'.

  ### get rid of one of the two
  Finally, the functions changedSinceLastRun() and warnChangedSinceLastRun()
  hardly belong here.  They are reimplemented in DeclResolver to check whether
  a piece of documentation has changed since the last qdoc run.
*/
class Resolver
{
public:
    Resolver() { }
    virtual ~Resolver() { }

    virtual QString resolve( const QString& name ) const;
    virtual bool changedSinceLastRun( const QString& link,
				      const QString& html ) const;
    virtual bool warnChangedSinceLastRun( const Location& loc,
					  const QString& link,
					  const QString& html ) const;
    virtual QString relatedProperty( const QString& name ) const;

    QString href( const QString& name,
		  const QString& text = QString::null ) const;
};

#endif

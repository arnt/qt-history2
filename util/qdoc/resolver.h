/*
  resolver.h
*/

#ifndef RESOLVER_H
#define RESOLVER_H

#include <qstring.h>

class Location;

/*
  The Resolver class is a base class for resolvers of identifiers.

  Member function resolve() returns a link to the documentation of an
  identifier.  For example, resolving 'QRegExp::search' in Qt should give
  'qregexp.html#search'.  Function resolvefn() does the same, except that the
  result is guaranteed to be a link to a function.  (This is for resolving
  ambiguity caused by constructors.)  Function href() is provided for
  convenience.  It returns a string of the style
  '<a href=qregexp.html#search>QRegExp::search</a>()'.

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
    virtual QString resolvefn( const QString& name ) const;
    virtual bool changedSinceLastRun( const QString& link,
				      const QString& html ) const;
    virtual void warnChangedSinceLastRun( const Location& loc,
					  const QString& link,
					  const QString& html ) const;

    QString href( const QString& name,
		  const QString& text = QString::null ) const;
};

#endif

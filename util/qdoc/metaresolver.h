/*
  metaresolver.h
*/

#ifndef METARESOLVER_H
#define METARESOLVER_H

#include <qmap.h>
#include <qstring.h>

#include "resolver.h"
#include "stringset.h"

/*
  The MetaResolver class is a Resolver that extends a base Resolver
  by providing new classes to the class hierarchy and new member
  functions.

  MetaResolver is used to document example code that contains
  definitions like

      class MyWidget : public QWidget {
	  // ...
	  void foo();
      };

  QWidget is part of Qt and it can be resolved using an ordinary
  DeclResolver. However, there's no room for MyWidget and other
  custom classes in that scheme, since they have no Decl object. The
  MetaResolver acts as a wrapper for the DeclResolver for Qt and
  takes care of MyWidget's members and its place in the hierarchy.

  MetaResolver provides two member functions not inherited from
  Resolver: setClassInheritanceMap() and setMemberFunctionMap().
  Here's how to use them:

      MetaResolver metaRes( res );
      QMap<QString, StringSet> cinherits;
      QMap<QString, QMap<QString, int> > mfunctions;

      cinherits[QString("MyWidget")].insert( QString("QWidget") );
      metaRes.setClassInheritanceMap( cinherits );

      mfunctions[QString("MyWidget")].insert( QString("foo"), "#f68" );
      metaRes.setMemberFunctionMap( mfunctions );

  Afterwards, 'foo' resolves as '#f68'.
*/
class MetaResolver : public Resolver
{
public:
    MetaResolver( const Resolver *res ) : r( res ) { }

    virtual QString resolve( const QString& name ) const;
    void setClassInheritanceMap( const QMap<QString, StringSet>& map )
    { cinherits = map; }
    void setMemberFunctionMap(
	    const QMap<QString, QMap<QString, QString> >& map )
    { mfunctions = map; }

private:
#if defined(Q_DISABLE_COPY)
    MetaResolver( const MetaResolver& );
    MetaResolver& operator=( const MetaResolver& );
#endif

    const Resolver *r;

    // QMap<subclass, superclasses>
    QMap<QString, StringSet> cinherits;

    // QMap<class, QMap<function, link> >
    QMap<QString, QMap<QString, QString> > mfunctions;
};

#endif

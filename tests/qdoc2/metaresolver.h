/*
  metaresolver.h
*/

#ifndef METARESOLVER_H
#define METARESOLVER_H

#include <qmap.h>
#include <qstring.h>

#include "resolver.h"
#include "stringset.h"

class MetaResolver : public Resolver
{
public:
    MetaResolver( const Resolver *res ) : r( res ) { }

    virtual QString resolve( const QString& name ) const;
    void setClassInheritanceMap( const QMap<QString, StringSet>& map )
    { cinherits = map; }
    void setMemberFunctionMap( const QMap<QString, QMap<QString, int> >& map )
    { mfunctions = map; }

private:
#if defined(Q_DISABLE_COPY)
    MetaResolver( const MetaResolver& );
    MetaResolver& operator=( const MetaResolver& );
#endif

    const Resolver *r;
    QMap<QString, StringSet> cinherits;
    QMap<QString, QMap<QString, int> > mfunctions;
};

#endif

/*
  property.cpp
*/

#include "property.h"

Property::Property()
    : store( Tdef ), design( Tdef )
{
}

Property::Property( const QString& type, const QString& name )
    : t( type ), n( name ), store( Tdef ), design( Tdef )
{
}

Property::Property( const Property& p )
    : t( p.t ), n( p.n ), read( p.read ), write( p.write ), store( p.store ),
      design( p.design ), reset( p.reset )
{
}

Property& Property::operator=( const Property& p )
{
    t = p.t;
    n = p.n;
    read = p.read;
    write = p.write;
    store = p.store;
    design = p.design;
    reset = p.reset;
    return *this;
}

Property::Trool Property::toTrool( bool b )
{
    return b ? Ttrue : Tfalse;
}

bool Property::fromTrool( Trool tr, bool def )
{
    return tr == Tdef ? def : tr == Ttrue;
}

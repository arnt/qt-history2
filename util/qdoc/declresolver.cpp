/*
  declresolver.cpp
*/

#include <stdlib.h>

#include "config.h"
#include "decl.h"
#include "declresolver.h"
#include "messages.h"

DeclResolver::DeclResolver( const Decl *root )
    : r( root ), c( 0 )
{
}

QString DeclResolver::resolve( const QString& name ) const
{
    const Decl *x = c != 0 ? c : r;
    const Decl *y = x->resolvePlain( name );

    if ( name.isEmpty() || (c != 0 && c->fullName() == name) ) {
	return QString::null;
    } else if ( y == 0 ) {
	if ( eg.contains(name) || h.contains(name) ) {
	    return config->verbatimHref( name );
	} else if ( html.contains(name) ) {
	    return name;
	} else {
	    return QString::null;
	}
    } else if ( y->kind() == Decl::Class ) {
	return config->classRefHref( y->fullName() );
    } else {
	if ( y->kind() == Decl::Function ) {
	    const Decl * z = y;
	    while ( z && z->internal() )
		z = z->reimplements();
	    if ( y->internal() && z && !z->internal() )
		y = z;
	    if ( y->internal() )
		return QString::null;
	}
	QString t;
	if ( y->relatesContext() != (Decl *) c )
	    t = config->classRefHref( y->relatesContext()->fullName() );
	return t + QChar( '#' ) + y->anchor();
    }
}

QString DeclResolver::resolvefn( const QString& name ) const
{
    const Decl *x = c != 0 ? c : r;
    const Decl *y = x->resolvePlain( name );

    if ( y == 0 || y->kind() != Decl::Function ) {
	return QString::null;
    } else {
	QString t;
	if ( y->relatesContext() != (Decl *) c )
	    t = config->classRefHref( y->relatesContext()->fullName() );
	return t + QChar( '#' ) + y->anchor();
    }
}

bool DeclResolver::changedSinceLastRun( const QString& link,
					const QString& html ) const
{
    QMap<QString, HtmlChunk>::ConstIterator chk = chkmap.find( link );
    return chk == chkmap.end() || !(*chk).isSame( html );
}

void DeclResolver::warnChangedSinceLastRun( const Location& loc,
					    const QString& link,
					    const QString& html ) const
{
    QMap<QString, HtmlChunk>::ConstIterator chk = chkmap.find( link );
    if ( chk == chkmap.end() ) {
	warning( 0, loc, "New documentation at %s%s (%d byte%s)",
		 config->base().latin1(), link.latin1(), html.length(),
		 html.length() == 1 ? "" : "s" );
    } else if ( !(*chk).isSame(html) ) {
	int delta = html.length() - (*chk).length();
	warning( 0, loc, "Modified documentation at %s%s (%+d byte%s)",
		 config->base().latin1(), link.latin1(), delta,
		 abs(delta) == 1 ? "" : "s" );
    }
}

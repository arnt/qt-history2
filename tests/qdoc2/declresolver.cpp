/*
  declresolver.cpp
*/

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
	if ( html.contains(name) )
	    return name;
	else if ( h.contains(name) )
	    return config->verbatimHref( name );
	else
	    return QString::null;
    } else if ( y->kind() == Decl::Class ) {
	return config->classRefHref( y->fullName() );
    } else {
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

void DeclResolver::compare( const Location& loc, const QString& link,
			    const QString& html ) const
{
    QMap<QString, HtmlChunk>::ConstIterator chk = chkmap.find( link );
    if ( chk == chkmap.end() ) {
	warning( 2, loc, "New documentation at %s%s (%d char%s)",
		 config->base().latin1(), link.latin1(), html.length(),
		 html.length() == 1 ? "" : "s" );
    } else if ( !(*chk).isSame(html) ) {
	int delta = html.length() - (*chk).length();
	warning( 2, loc, "Modified documentation at %s%s (%+d char%s)",
		 config->base().latin1(), link.latin1(), delta,
		 (delta == 1 || delta == -1) ? "" : "s" );
    }
}

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
	} else {
	    QString base = name;
	    QString ref;

	    int k = base.find( QChar('#') );
	    if ( k != -1 ) {
		base.truncate( k );
		ref = name.right( k );
	    }

	    if ( html.contains(base) ) {
		return name;
	    } else if ( base.endsWith(QString(".book")) ) {
		// ### links may be broken
		return base.left( base.length() - 5 ) + QString( ".html" ) +
		       ref;
	    } else {
		return QString::null;
	    }
	}
    } else if ( y->kind() == Decl::Class ) {
	return config->classRefHref( y->fullName() );
    } else {
	return resolved( y );
    }
}

bool DeclResolver::changedSinceLastRun( const QString& link,
					const QString& html ) const
{
    QMap<QString, HtmlChunk>::ConstIterator chk = chkmap.find( link );
    return chk == chkmap.end() || !(*chk).isSame( html );
}

bool DeclResolver::warnChangedSinceLastRun( const Location& loc,
					    const QString& link,
					    const QString& html ) const
{
    QMap<QString, HtmlChunk>::ConstIterator chk = chkmap.find( link );
    if ( chk == chkmap.end() ) {
	warning( 0, loc, "New documentation at %s%s (%d byte%s)",
		 config->base().latin1(), link.latin1(), html.length(),
		 html.length() == 1 ? "" : "s" );
	return TRUE;
    } else if ( !(*chk).isSame(html) ) {
	int delta = html.length() - (*chk).length();
	warning( 0, loc, "Editorial change at %s%s (%+d byte%s)",
		 config->base().latin1(), link.latin1(), delta,
		 abs(delta) == 1 ? "" : "s" );
	return TRUE;
    } else {
	return FALSE;
    }
}

QString DeclResolver::relatedProperty( const QString& name ) const
{
    const Decl *x = c != 0 ? c : r;
    const Decl *y = x->resolvePlain( name );

    if ( y != 0 && y->kind() == Decl::Function ) {
	FunctionDecl *func = (FunctionDecl *) y;
	if ( func->relatedProperty() != 0 ) {
	    QString prefix;
	    int k = name.findRev( QChar(':') );
	    if ( k != -1 )
		prefix = name.left( k + 1 );
	    return prefix + func->relatedProperty()->name();
	}
    }
    return QString::null;
}

QString DeclResolver::resolved( const Decl *decl ) const
{
    if ( decl->kind() == Decl::Function ) {
	while ( decl != 0 && decl->isInternal() )
	    decl = decl->reimplements();
    }

    QString t;
    if ( decl == 0 || decl->relatesContext() == r )
	return QString::null;
    if ( decl->relatesContext() != (Decl *) c )
	t = config->classRefHref( decl->relatesContext()->fullName() );
    return t + QChar( '#' ) + decl->ref();
}

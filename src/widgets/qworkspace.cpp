/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qworkspace.cpp#8 $
**
** Implementation of the QWorkspace class
**
** Created : 931107
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/
#include "qworkspace.h"
#include <qapplication.h>
#include <qobjectlist.h>
#include "qworkspacechild.h"

//
//  W A R N I N G
//  -------------
//
//  It is very unlikely that this code will be available in the final
//  Qt 2.0 release.  It will be available soon after then, but a number
//  of important API changes still need to be made.
//
//  Thus, it is important that you do NOT use this code in an application
//  unless you are willing for your application to be dependent on the
//  snapshot releases of Qt.
//

#define OFFSET 20

QWorkspace::QWorkspace( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    active = 0;

    px = 0;
    py = 0;
    
    maxClient = 0;

}

QWorkspace::~QWorkspace()
{
}


void QWorkspace::childEvent( QChildEvent * e)
{

    if (e->inserted() && e->child()->isWidgetType()) {
	QWidget* w = (QWidget*) e->child();
	if ( w->testWFlags( WStyle_Customize | WStyle_NoBorder )
	      || icons.contains( w ) )
	    return; 	    // nothing to do
	
	bool doShow = w->isVisible();
	
	QWorkspaceChild* child = new QWorkspaceChild( w, this );
	windows.append( child );
	place( child );
	child->raise();
	if ( TRUE || doShow ) {
	  child->show();
	  activateClient( w );
	}
    } else if (e->removed() ) {
	if ( maxClient == e->child() ) {
	    maxClient = 0;
	    hideMaxHandles();
	}
	if ( windows.contains( (QWorkspaceChild*)e->child() ) )
	    windows.remove( (QWorkspaceChild*)e->child() );
	if ( icons.contains( (QWidget*)e->child() ) ){
	    icons.remove( (QWidget*)e->child() );
	    layoutIcons();
	}
    }
}


void QWorkspace::activateClient( QWidget* w)
{
    for (QWorkspaceChild* c = windows.first(); c; c = windows.next() ) {
	c->setActive( c->clientWidget() == w );
	if (c->clientWidget() == w)
	    active = c;
    }

    if (!active)
	return;

    active->raise();

    QObjectList* ol = active->queryList( "QWidget" );
    bool hasFocus = FALSE;
    for (QObject* o = ol->first(); o; o = ol->next() ) {
	hasFocus |= ((QWidget*)o)->hasFocus();
    }
    if ( !hasFocus ) {
	active->clientWidget()->setFocus();
    }
    delete ol;

    emit clientActivated( w );
}


QWidget* QWorkspace::activeClient() const
{
    return active;
}



void QWorkspace::place( QWorkspaceChild* c)
{
    int tx,ty;

    QRect maxRect = rect();
    if (px < maxRect.x())
	px = maxRect.x();
    if (py < maxRect.y())
	py = maxRect.y();

    px += OFFSET;
    py += 2*OFFSET;

    if (px > maxRect.width()/2)
	px =  maxRect.x() + OFFSET;
    if (py > maxRect.height()/2)
	py =  maxRect.y() + OFFSET;
    tx = px;
    ty = py;
    if (tx + c->width() > maxRect.right()){
	tx = maxRect.right() - c->width();
	if (tx < 0)
	    tx = 0;
	px =  maxRect.x();
    }
    if (ty + c->height() > maxRect.bottom()){
	ty = maxRect.bottom() - c->height();
	if (ty < 0)
	    ty = 0;
	py =  maxRect.y();
    }
    c->move( tx, ty );
}

void QWorkspace::insertIcon( QWidget* w )
{
    if (icons.contains(w) )
	return;
    icons.append( w );
    if (w->parentWidget() != this )
	w->reparent( this, 0, QPoint(0,0), FALSE);
    layoutIcons();
    if (isVisible())
	w->show();

}

void QWorkspace::removeIcon( QWidget* w)
{
    if (!icons.contains( w ) )
	return;
    icons.remove( w );
    w->hide();
 }

void QWorkspace::resizeEvent( QResizeEvent * )
{
    layoutIcons();
}

void QWorkspace::layoutIcons()
{
    int x = 0;
    int y = height();
    for (QWidget* w = icons.first(); w ; w = icons.next() ) {
	
	if ( x > 0 && x + w->width() > width() ){
	    x = 0;
	    y -= w->height();
	}
	
	w->move(x, y-w->height());
	x = w->geometry().right();
    }
}

void QWorkspace::minimizeClient( QWidget* w)
{
    QWorkspaceChild* c = findChild( w );
    if ( c ) {
	c->hide();
	insertIcon( c->iconWidget() );
	if ( c == maxClient ) {
	    c->setGeometry( maxRestore );
	    maxClient = 0;
	    hideMaxHandles();
	}
	    
    }
}

void QWorkspace::normalizeClient( QWidget* w)
{
    QWorkspaceChild* c = findChild( w );
    if ( c ) {
	if ( c == maxClient ) {
	    c->setGeometry( maxRestore );
	}
	else {
	    removeIcon( c->iconWidget() );
	    c->show();
	}
    }
}

void QWorkspace::maximizeClient( QWidget* w)
{
    QWorkspaceChild* c = findChild( w );
    if (maxClient == c) {
	normalizeClient( w ); // hack for now, should not toggle
	return;
    }
    if ( c ) {
	if (icons.contains(c->iconWidget()) )
	    normalizeClient( w );
	maxRestore = c->geometry();
	c->setGeometry( rect() );
	maxClient = c;
	c->show();
    }
}

QWorkspaceChild* QWorkspace::findChild( QWidget* w)
{
    for (QWorkspaceChild* c = windows.first(); c; c = windows.next() ) {
	c->setActive( c->clientWidget() == w );
	if (c->clientWidget() == w)
	    return c;
    }
    return 0;
}

void QWorkspace::showMaxHandles()
{
}

void QWorkspace::hideMaxHandles()
{
}

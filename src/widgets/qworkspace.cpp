/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qworkspace.cpp#2 $
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

QWorkspace::QWorkspace( QWidget *parent=0, const char *name=0 )
    : QWidget( parent, name )
{
    manage = TRUE;
    active = 0;
    
    px = 0;
    py = 0;

}

QWorkspace::~QWorkspace()
{
}


/*!
  Returns whether the workspace automatically manages new child
  windows. The default is TRUE.
 */
bool QWorkspace::autoManage()
{
    return manage;
}

/*!
  Sets whether the workspace should automatically manages new child
  windows. The default is TRUE.
 */
void QWorkspace::setAutoManage( bool m)
{
    manage = m;
}


void QWorkspace::childEvent( QChildEvent * e)
{

    if (e->type() == QEvent::ChildInserted && e->child()->isWidgetType()) {
	QWidget* w = (QWidget*) e->child();
	if ( !manage || w->inherits("QWorkspaceChild") )
	    return; 	    // already here
	
	bool doShow = w->isVisible();
	QWorkspaceChild* child = new QWorkspaceChild( w, this );
	windows.append( child );
	place( child );
	if ( doShow )
	    child->show();
	activateClient( w );
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
    QObjectList* ol = active->queryList( "QWidget" );
    bool hasFocus = FALSE;
    for (QObject* o = ol->first(); o; o = ol->next() ) {
	hasFocus |= ((QWidget*)o)->hasFocus();
    }
    if ( !hasFocus ) {
	active->clientWidget()->setFocus();
    }
    delete ol;
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

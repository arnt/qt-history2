/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qdropsite.cpp#1 $
**
** Implementation of Drag and Drop support
**
** Copyright (C) 1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qdropsite.h"
#include "qwidget.h"

class QDropSitePrivate : public QObject {
    QDropSite* s;

public:
    QDropSitePrivate( QWidget* parent, QDropSite* site ) :
	QObject(parent),
	s(site)
    {
	parent->installEventFilter(this);
    }

    bool eventFilter( QObject*, QEvent* );
};

bool QDropSitePrivate::eventFilter( QObject *, QEvent * e )
{
    if ( e->type() == Event_Drop ) {
	s->dropEvent( (QDropEvent *)e );
	return TRUE;
    } else if ( e->type() == Event_DragEnter ) {
	s->dragEnterEvent( (QDragEnterEvent *)e );
	return TRUE;
    } else if ( e->type() == Event_DragMove ) {
	s->dragMoveEvent( (QDragMoveEvent *)e );
	return TRUE;
    } else if ( e->type() == Event_DragLeave ) {
	s->dragLeaveEvent( (QDragLeaveEvent *)e );
	return TRUE;
    } else {
	return FALSE;
    }
}


QDropSite::QDropSite( QWidget* parent ) :
    d(new QDropSitePrivate(parent,this))
{
    parent->setAcceptDrops( TRUE );
}

QDropSite::~QDropSite()
{
    delete d; // not really needed
}

void QDropSite::dragEnterEvent( QDragEnterEvent * )
{
}

void QDropSite::dragMoveEvent( QDragMoveEvent * )
{
}

void QDropSite::dragLeaveEvent( QDragLeaveEvent * )
{
}

void QDropSite::dropEvent( QDropEvent * )
{
}

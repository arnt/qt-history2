/****************************************************************************
** $Id: $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qheader.h>
#include <qtable.h>

#include "whatsthis.h"

WhatsThis::WhatsThis( QWidget *w, QWidget *watch )
: QWhatsThis( watch ? watch : w ), widget( w )
{
}

QWidget *WhatsThis::parentWidget() const
{
    return widget;
}

bool WhatsThis::clicked( const QString & href )
{
    if ( !href.isEmpty() )
	emit linkClicked( href );

    return TRUE;
}

HeaderWhatsThis::HeaderWhatsThis( QHeader *h ) 
: WhatsThis( h )
{
}

QString HeaderWhatsThis::text( const QPoint &p )
{
    QHeader *header = (QHeader*)parentWidget();

    QString orient = (header->orientation() == QObject::Horizontal) ? "horizontal" : "vertical";
    return QString("This is the %1 <a href=%2/html/qheader.html>header</a>.").
	arg(orient).
	arg(qInstallPathDocs());
}

TableWhatsThis::TableWhatsThis( QTable *t ) 
: WhatsThis( t, t->viewport() )
{
}

QString TableWhatsThis::text( const QPoint &p )
{
    QTable *table = (QTable*)parentWidget();

    QPoint cp = table->viewportToContents( p );
    int r = table->rowAt( cp.y() );
    int c = table->columnAt( cp.x() );

    QTableItem* i = table->item( r,c  );
    if ( !i )
	return "This is empty space.";

    switch ( i->rtti() ) {
    case 0:
	return QString("This is a <a href=%1/html/qtableitem.html>QTableItem</a>.").
		       arg(qInstallPathDocs());
    case 1:
	return QString("This is a <a href=%1/html/qcombotableitem.html>QComboTableItem</a>."
		       "<br>It can be used to provide multiple-choice items in a table.").
		       arg(qInstallPathDocs());
    case 2:
	return QString("This is a <a href=%1/html/qchecktableitem.html>QCheckTableItem</a>."
		       "<br>It provide <a href=%1/html/qcheckbox.html>checkboxes</a> in tables.").
		       arg(qInstallPathDocs()).arg(qInstallPathDocs());
    default:
	break;
    }

    return "This is a user defined table item.";
}

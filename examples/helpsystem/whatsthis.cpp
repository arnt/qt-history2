/****************************************************************************
** $Id: $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qapplication.h>
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

QString HeaderWhatsThis::text( const QPoint & )
{
    QHeader *header = (QHeader*)parentWidget();

    QString orient = (header->orientation() == QObject::Horizontal) ? "horizontal" : "vertical";
    QString docsPath = qApp->applicationDirPath() + "/../../doc";
    return QString("This is the %1 <a href=%2/html/qheader.html>header</a>.").
	arg(orient).
	arg(docsPath);
}

TableWhatsThis::TableWhatsThis( QTable *t ) 
: WhatsThis( t, t->viewport() )
{
}


QString TableWhatsThis::text( const QPoint &p )
{
    QTable *table = (QTable*)parentWidget();

    QPoint cp = table->viewportToContents( p );
    int row = table->rowAt( cp.y() );
    int col = table->columnAt( cp.x() );

    QTableItem* i = table->item( row,col  );
    if ( !i )
	return "This is empty space.";

    QString docsPath = qApp->applicationDirPath() + "/../../doc";

    switch ( i->rtti() ) {
    case 0: //QTableItem::RTTI
	return QString("This is a <a href=%1/html/qtableitem.html>QTableItem</a>.").
		       arg(docsPath);
    case 1: //QComboTableItem::RTTI
	return QString("This is a <a href=%1/html/qcombotableitem.html>QComboTableItem</a>."
		       "<br>It can be used to provide multiple-choice items in a table.").
		       arg(docsPath);
    case 2: //QCheckTableItem::RTTI
	return QString("This is a <a href=%1/html/qchecktableitem.html>QCheckTableItem</a>."
		       "<br>It provide <a href=%1/html/qcheckbox.html>checkboxes</a> in tables.").
		       arg(docsPath).arg(docsPath);
    default:
	break;
    }

    return "This is a user defined table item.";
}

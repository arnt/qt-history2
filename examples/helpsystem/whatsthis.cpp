/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

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

bool WhatsThis::clicked( const QString &link )
{
    if ( !link.isEmpty() )
	emit linkClicked( link );

    return TRUE;
}

HeaderWhatsThis::HeaderWhatsThis( QHeader *h ) 
: WhatsThis( h )
{
}

QString HeaderWhatsThis::text( const QPoint &p )
{
    QHeader *header = (QHeader*)parentWidget();

    QString orient;
    int section;
    if ( header->orientation() == QObject::Horizontal ) {
	orient = "horizontal";
	section = header->sectionAt( p.x() );
    } else {
	orient = "vertical";
	section = header->sectionAt( p.y() );
    }
    if( section == -1 )
	return "This is empty space.";
    QString docsPath = qApp->applicationDirPath() + "/../../doc";
    return QString("This is the %1. section in the %2 <a href=%2/html/qheader.html>header</a>.").
	arg(section + 1).
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

    if ( row == -1 || col == -1 )
	return "This is empty space.";

    QTableItem* i = table->item( row,col  );
    if ( !i )
	return "This is an empty cell.";

    QString docsPath = qApp->applicationDirPath() + "/../../doc";

    if ( QTableItem::RTTI == i->rtti() ) {
	return QString("This is a <a href=%1/html/qtableitem.html>QTableItem</a>.").
		       arg(docsPath);
    } else if ( QComboTableItem::RTTI == i->rtti() ) {
	return QString("This is a <a href=%1/html/qcombotableitem.html>QComboTableItem</a>."
		       "<br>It can be used to provide multiple-choice items in a table.").
		       arg(docsPath);
    } else if ( QCheckTableItem::RTTI == i->rtti() ) {
	return QString("This is a <a href=%1/html/qchecktableitem.html>QCheckTableItem</a>."
		       "<br>It provide <a href=%1/html/qcheckbox.html>checkboxes</a> in tables.").
		       arg(docsPath).arg(docsPath);
    }

    return "This is a user defined table item.";
}

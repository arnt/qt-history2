/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qtooltip.h>
#include <qtable.h>

#include "tooltip.h"

HeaderToolTip::HeaderToolTip( QHeader *header, QToolTipGroup *group ) 
: QToolTip( header, group )
{
}

void HeaderToolTip::maybeTip ( const QPoint& p )
{
    QHeader *header = (QHeader*)parentWidget();
    
    int section = 0;
    
    if ( header->orientation() == Horizontal )
	section= header->sectionAt( p.x() );
    else
	section= header->sectionAt( p.y() );
    
    QString tipString = header->label( section );
    tip( header->sectionRect( section ), tipString, "This is a section in a header" );
}

TableToolTip::TableToolTip( QTable *tipTable, QToolTipGroup *group ) 
: QToolTip( tipTable->viewport(), group ), table( tipTable )
{
}


void TableToolTip::maybeTip ( const QPoint &p )
{
    QPoint cp = table->viewportToContents( p );
    int row = table->rowAt( cp.y() );
    int col = table->columnAt( cp.x() );
   
    QString tipString = table->text( row, col );

    QRect cr = table->cellGeometry( row, col );
    cr.moveTopLeft( table->contentsToViewport( cr.topLeft() ) );
    tip( cr, tipString, "This is a cell in a table" );
}

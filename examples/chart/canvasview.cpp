/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "canvasview.h"
#include "chartform.h"

#include <qcursor.h>
#include <qpoint.h>
#include <qpopupmenu.h>
#include <qstatusbar.h>
#include <qevent.h>

void CanvasView::contentsContextMenuEvent( QContextMenuEvent * )
{
    ((ChartForm*)parent())->optionsMenu->exec( QCursor::pos() );
}


void CanvasView::viewportResizeEvent( QResizeEvent *e )
{
    canvas()->resize( e->size().width(), e->size().height() );
    ((ChartForm*)parent())->drawElements();
}


void CanvasView::contentsMousePressEvent( QMouseEvent *e )
{
    QCanvasItemList list = canvas()->collisions( e->pos() );
    for ( QCanvasItemList::iterator it = list.begin(); it != list.end(); ++it )
	if ( (*it)->rtti() == CanvasText::CANVAS_TEXT ) {
	    m_movingItem = *it;
	    m_pos = e->pos();
	    return;
	}
    m_movingItem = 0;
}


void CanvasView::contentsMouseMoveEvent( QMouseEvent *e )
{
    if ( m_movingItem ) {
	QPoint offset = e->pos() - m_pos;
	m_movingItem->moveBy( offset.x(), offset.y() );
	m_pos = e->pos();
	ChartForm *form = (ChartForm*)parent();
	form->setChanged( TRUE );
	int chartType = form->chartType();
	CanvasText *item = (CanvasText*)m_movingItem;
	int i = item->index();

	(*m_elements)[i].setProX( chartType, item->x() / canvas()->width() );
	(*m_elements)[i].setProY( chartType, item->y() / canvas()->height() );

	canvas()->update();
    }
}



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

#include <qdragobject.h>
#include <qevent.h>
#include "dnd.h"
#include "iconview.h"


IconView::IconView( QWidget* parent, const char* name )
    : QIconView( parent, name )
{
    connect( this, SIGNAL(dropped(QDropEvent*, const QValueList<QIconDragItem>&)),
             SLOT(slotNewItem(QDropEvent*, const QValueList<QIconDragItem>&)));
}

IconView::~IconView()
{

}


QDragObject *IconView::dragObject()
{
    if ( !currentItem() ) return 0;

    QTextDrag * drg = new QTextDrag( ((IconViewItem*)currentItem())->tag(), this );
    drg->setSubtype("dragdemotag");
    drg->setPixmap( *currentItem()->pixmap() );

    return drg;
}

void IconView::slotNewItem( QDropEvent *e, const QValueList<QIconDragItem>& )
{
    QString tag;
    if ( !e->provides( "text/dragdemotag" ) ) return;

    if ( QTextDrag::decode( e, tag ) ) {
        IconItem item = ((DnDDemo*) parentWidget())->findItem( tag );
        IconViewItem *iitem = new IconViewItem( this, item.name(), *item.pixmap(), tag );
        iitem->setRenameEnabled( TRUE );
    }
    e->acceptAction();
}

#include <iostream.h>
#include <qdragobject.h>

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

/*
QDragObject *IconView::dragObject()
{
    if ( !currentItem() ) return 0;
    
    QTextDrag * drg = new QTextDrag( currentItem()->text(), this );
    drg->setPixmap( *currentItem()->pixmap() );
    
    return drg;
}
*/

void IconView::slotNewItem( QDropEvent *evt, const QValueList<QIconDragItem>& )
{
    QString label;

    if ( QTextDrag::decode( evt, label ) ) {
        QIconViewItem *item = new QIconViewItem( this, label );
        item->setRenameEnabled( TRUE );
    }
}

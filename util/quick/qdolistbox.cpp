#include "qdolistbox.h"

QDragOffListBox::QDragOffListBox(QWidget* parent=0, const char* name=0, WFlags f=0) :
    QListBox(parent,name,f)
{
    loaded = TRUE;
    setDragSelect( FALSE );
}

void QDragOffListBox::mousePressEvent (QMouseEvent* e)
{
    QListBox::mousePressEvent( e );
    loaded = FALSE;
}

void QDragOffListBox::mouseMoveEvent (QMouseEvent* e)
{
    QListBox::mouseMoveEvent( e );
    if ( !loaded ) {
	loaded = TRUE;
	emit dragged( text( currentItem() ) );
	emit dragged( currentItem() );
    }
}



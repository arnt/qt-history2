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

#include "listboxrename.h"
#include <qheader.h>
#include <qlineedit.h>
#include <qtimer.h>
#include <qevent.h>

class EditableListBoxItem : public QListBoxItem
{
public:
    void setText( const QString & text )
    {
	QListBoxItem::setText( text );
    }
};

ListBoxRename::ListBoxRename( QListBox * eventSource, const char * name )
    : QObject( eventSource, name ),
      clickedItem( 0 ), activity( false )
{
    src = eventSource;
    src->installEventFilter( this );
    ed = new QLineEdit( src->viewport() );
    ed->hide();
    ed->setFrame( false );

    QObject::connect( ed, SIGNAL( returnPressed() ),
		      this, SLOT( renameClickedItem() ) );
}

bool ListBoxRename::eventFilter( QObject *, QEvent * event )
{
    switch ( event->type() ) {

    case QEvent::MouseButtonPress:
        {
	    QPoint pos = ((QMouseEvent *) event)->pos();

	    if ( clickedItem &&
		 clickedItem->isSelected() &&
		 (clickedItem == src->itemAt( pos )) ) {
		QTimer::singleShot( 500, this, SLOT( showLineEdit() ) );
		activity = false; // no drags or clicks for 500 ms before we start the renaming
	    } else { // new item clicked
		activity = true;
		clickedItem = src->itemAt( pos );
		ed->hide();
	    }
	}
        break;

    case QEvent::MouseMove:

	if ( ((QMouseEvent *) event)->state() & Qt::LeftButton ) {
	    activity = true;  // drag
	}
	break;

    case QEvent::KeyPress:

	switch ( ((QKeyEvent *) event)->key() ) {

	case Qt::Key_F2:

	    activity = false;
	    clickedItem = src->item( src->currentItem() );
	    showLineEdit();
	    break;

	case Qt::Key_Escape:
	    if ( !ed->isHidden() ) {
		hideLineEdit(); // abort rename
		return true;
	    }
	    break;

	case Qt::Key_Up:
	case Qt::Key_Down:
	case Qt::Key_PageUp:
	case Qt::Key_PageDown:

	    if ( !ed->isHidden() )
		return true; // Filter out the keystrokes
	    break;

	}
	break;

    case QEvent::Resize:

	if ( clickedItem && ed && !ed->isHidden() ) {
	    QRect rect = src->itemRect( clickedItem );
	    ed->resize( rect.right() - rect.left() - 1,
		rect.bottom() - rect.top() - 1 );
	}
	break;

    default:
	break;
    }

    return false;
}

void ListBoxRename::showLineEdit()
{
    if ( !clickedItem || activity )
	return;
    QRect rect = src->itemRect( clickedItem );
    ed->resize( rect.right() - rect.left() - 1,
		rect.bottom() - rect.top() - 1 );
    ed->move( rect.left() + 1, rect.top() + 1 );
    ed->setText( clickedItem->text() );
    ed->selectAll();
    ed->show();
    ed->setFocus();
}

void ListBoxRename::hideLineEdit()
{
    ed->hide();
    clickedItem = 0;
    src->setFocus();
}

void ListBoxRename::renameClickedItem()
{
    if ( clickedItem && ed ) {
	( (EditableListBoxItem *) clickedItem )->setText( ed->text() );
	emit itemTextChanged( ed->text() );
    }
    hideLineEdit();
}

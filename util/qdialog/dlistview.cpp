#include "dlistview.h"
#include "dlistviewedit.h"

#include <qpoint.h>
#include <qpopupmenu.h>
#include <qheader.h>
#include <qresource.h>

DListView::DListView( QWidget* parent, const QResource& resource )
  : QListView( parent, resource )
{
  connect( this, SIGNAL( rightButtonPressed( QListViewItem*, const QPoint&, int ) ),
	   this, SLOT( slotRightButtonPressed( QListViewItem*, const QPoint&, int ) ) );
}

void DListView::slotRightButtonPressed( QListViewItem *item, const QPoint& point, int col )
{
  m_item = item;
  m_column = col;

  QPopupMenu* menu = new QPopupMenu( this );
  if ( m_item )
  {
    menu->insertItem( tr("Insert before"), this, SLOT( slotInsertBefore() ) );
    menu->insertItem( tr("Insert after"), this, SLOT( slotInsertAfter() ) );
    menu->insertItem( tr("Insert child"), this, SLOT( slotInsertChild() ) );
  }
  else
    menu->insertItem( tr("Insert new item"), this, SLOT( slotInsert() ) );
  menu->insertSeparator();
  menu->insertItem( tr("Remove"), this, SLOT( slotRemove() ) );
  menu->insertSeparator();
  menu->insertItem( tr("Edit"), this, SLOT( slotEdit() ) );

  menu->popup( point );
}

void DListView::slotInsertAfter()
{
  int cols = columns();
  QListViewItem* item = 0;
  if ( m_item->parent() )
    item = new QListViewItem( m_item->parent(), m_item );
  else
    item = new QListViewItem( this, m_item );

  for( int i = 0; i < cols; ++i )
    item->setText( i, "Item" );
}

void DListView::slotInsertBefore()
{
  int cols = columns();
  QListViewItem* item = 0;

  QListViewItem* above = m_item->itemAbove();
  if ( above && above->nextSibling() == m_item )
  {
    m_item = above;
    slotInsertAfter();
    return;
  }

  if ( m_item->parent() )
    item = new QListViewItem( m_item->parent(), m_item );
  else
    item = new QListViewItem( this, m_item );

  for( int i = 0; i < cols; ++i )
    item->setText( i, "Item" );
}

void DListView::slotInsertChild()
{
  int cols = columns();
  QListViewItem* item = new QListViewItem( m_item );

  for( int i = 0; i < cols; ++i )
    item->setText( i, "Item" );

  m_item->setOpen( TRUE );
}

void DListView::slotRemove()
{
  if ( m_item->parent() )
    m_item->parent()->removeItem( m_item );
  else
    removeItem( m_item );
}

void DListView::slotEdit()
{
  // ### HACK correct path here
  QResource resource( "dlistviewedit.qdl" );
  DListViewEdit dlg( 0, resource );
  dlg.setText( m_item->text( m_column ) );
  if ( dlg.exec() )
  {
    m_item->setText( m_column, dlg.text() );
    if ( dlg.pixmap() )
      m_item->setPixmap( m_column, *(dlg.pixmap()) );
  }
}

void DListView::slotInsert()
{
  int cols = columns();
  QListViewItem* item = new QListViewItem( this );

  for( int i = 0; i < cols; ++i )
    item->setText( i, "Item" );
}

int DListView::columns()
{
  if ( header() )
    return header()->count();
  return 0;
}

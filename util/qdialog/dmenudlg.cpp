#define INCLUDE_MENUITEM_DEF

#include "dmenudlg.h"

#include <qpoint.h>
#include <qpopupmenu.h>
#include <qheader.h>
#include <qresource.h>
#include <qfiledialog.h>
#include <qpushbutton.h>
#include <qapplication.h>
#include <qkeycode.h>
#include <qmenubar.h>
#include <qmenudata.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>

/**********************************************************
 *
 * DMenuDlg
 *
 **********************************************************/

DMenuDlg::DMenuDlg( QMenuBar* bar, QWidget* parent, const QResource& resource )
  : DMenuDlg_skel( parent, resource )
{
  m_bar = bar;

  initListView( m_bar );
}

void DMenuDlg::initListView( QMenuData* mb, QListViewItem* parent )
{
  int count = mb->count();
  QListViewItem* lv = 0;
  for( int i = 0; i < count; ++i )
  {
    int id = mb->idAt( i );
    QMenuItem* item = mb->findItem( id );
    ASSERT( item );

    if ( parent )
    {
      if ( lv )
	lv = new QListViewItem( parent, lv );
      else
	lv = new QListViewItem( parent );
    }
    else
    {
      if ( lv )
	lv = new QListViewItem( lv_menu, lv );
      else
	lv = new QListViewItem( lv_menu );
    }

    if ( !item->text().isEmpty() )
      lv->setText( 0, item->text() );
    if ( item->pixmap() )
      lv->setPixmap( 0, *item->pixmap() );

    if ( item->popup() )
      initListView( item->popup(), lv );
  }
}

void DMenuDlg::slotIcon()
{
  QString fn = QFileDialog::getOpenFileName();
  if ( fn.isEmpty() )
    return;
  
  QPixmap pix( fn );
  if ( pix.isNull() )
  {
    QMessageBox::critical( this, tr("Loading Error"), tr("Could not load pixmap") );
    return;
  }
  
  pb_icon->setPixmap( pix ); 
}

void DMenuDlg::slotShortcut( bool _selected )
{
  if ( _selected )
    qApp->installEventFilter( this );
}

bool DMenuDlg::eventFilter( QObject*, QEvent* _ev )
{
  // The kind of events we are interested in ...
  if ( _ev->type() == QEvent::KeyPress )
  {
    QKeyEvent* ev = (QKeyEvent*)_ev;

    if ( ( ev->key() & (Key_Shift | Key_Control | Key_Alt | Key_Meta ) ) == ev->key() )
      return TRUE;

    qApp->removeEventFilter( this );

    int k = ev->key();
    if ( ev->state() & ShiftButton )
      k |= SHIFT;
    if ( ev->state() & ControlButton )
      k |= CTRL;
    if ( ev->state() & AltButton )
      k |= ALT;

    pb_shortcut->setText( accelString( k ) );
    pb_shortcut->setOn( FALSE );
    return TRUE;
  }

  return FALSE;
}

QString DMenuDlg::accelString( int k )
{
    QString s;
    if ( (k & SHIFT) == SHIFT )
	s = tr( "Shift" );
    if ( (k & CTRL) == CTRL ) {
	if ( !s.isEmpty() )
	    s += tr( "+" );
	s += tr( "Ctrl" );
    }
    if ( (k & ALT) == ALT ) {
	if ( !s.isEmpty() )
	    s += tr( "+" );
	s += tr( "Alt" );
    }
    k &= ~(SHIFT | CTRL | ALT);
    QString p;
    if ( (k & ASCII_ACCEL) == ASCII_ACCEL ) {
	k &= ~ASCII_ACCEL;
	p.sprintf( "%c", (k & 0xff) );
    } else if ( k >= Key_F1 && k <= Key_F24 ) {
	p = tr( "F%1" ).arg(k - Key_F1 + 1);
    } else if ( k > Key_Space && k <= Key_AsciiTilde ) {
	p.sprintf( "%c", k );
    } else {
	switch ( k ) {
	    case Key_Space:
		p = tr( "Space" );
		break;
	    case Key_Escape:
		p = tr( "Esc" );
		break;
	    case Key_Tab:
		p = tr( "Tab" );
		break;
	    case Key_Backtab:
		p = tr( "Backtab" );
		break;
	    case Key_Backspace:
		p = tr( "Backspace" );
		break;
	    case Key_Return:
		p = tr( "Return" );
		break;
	    case Key_Enter:
		p = tr( "Enter" );
		break;
	    case Key_Insert:
		p = tr( "Ins" );
		break;
	    case Key_Delete:
		p = tr( "Del" );
		break;
	    case Key_Pause:
		p = tr( "Pause" );
		break;
	    case Key_Print:
		p = tr( "Print" );
		break;
	    case Key_SysReq:
		p = tr( "SysReq" );
		break;
	    case Key_Home:
		p = tr( "Home" );
		break;
	    case Key_End:
		p = tr( "End" );
		break;
	    case Key_Left:
		p = tr( "Left" );
		break;
	    case Key_Up:
		p = tr( "Up" );
		break;
	    case Key_Right:
		p = tr( "Right" );
		break;
	    case Key_Down:
		p = tr( "Down" );
		break;
	    case Key_Prior:
		p = tr( "PgUp" );
		break;
	    case Key_Next:
		p = tr( "PgDown" );
		break;
	    case Key_CapsLock:
		p = tr( "CapsLock" );
		break;
	    case Key_NumLock:
		p = tr( "NumLock" );
		break;
	    case Key_ScrollLock:
		p = tr( "ScrollLock" );
		break;
	    default:
		p.sprintf( "<%d?>", k );
		break;
	}
    }
    if ( s.isEmpty() )
	s = p;
    else {
	s += tr( "+" );
	s += p;
    }
    return s;
}

/**********************************************************
 *
 * DMenuListView
 *
 **********************************************************/

DMenuListView::DMenuListView( QWidget* parent, const QResource& resource )
  : QListView( parent, resource )
{
  setSorting( -1 );

  connect( this, SIGNAL( rightButtonPressed( QListViewItem*, const QPoint&, int ) ),
	   this, SLOT( slotRightButtonPressed( QListViewItem*, const QPoint&, int ) ) );
}

void DMenuListView::slotRightButtonPressed( QListViewItem *item, const QPoint& point, int )
{
  m_item = item;

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

  menu->popup( point );
}

void DMenuListView::slotInsertAfter()
{
  QListViewItem* item = 0;
  if ( m_item->parent() )
    item = new QListViewItem( m_item->parent(), m_item );
  else
    item = new QListViewItem( this, m_item );

  item->setText( 0, "Item" );
}

void DMenuListView::slotInsertBefore()
{
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

  item->setText( 0, "Item" );
}

void DMenuListView::slotInsertChild()
{
  QListViewItem* item = new QListViewItem( m_item );

  item->setText( 0, "Item" );

  m_item->setOpen( TRUE );
}

void DMenuListView::slotRemove()
{
  if ( m_item->parent() )
    m_item->parent()->removeItem( m_item );
  else
    removeItem( m_item );
}

void DMenuListView::slotInsert()
{
  QListViewItem* item = new QListViewItem( this );

  item->setText( 0, "Item" );
}

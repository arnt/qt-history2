#include "widgetsbar.h"

#include <qtabbar.h>
#include <qlayout.h>
#include <qwidgetstack.h>
#include <qlabel.h>

#include <stdlib.h>

#define Dnd_X_Precision 2
#define Dnd_Y_Precision 2

/**********************************************
 *
 * DWidgetDrag
 *
 **********************************************/

DWidgetDrag::DWidgetDrag( const QString& _className, const QSize& _size, QWidget* _drag_source, const char* _name )
  : QDragObject( _drag_source, _name )
{
  data.className = _className;
  data.sizeHint = _size;
}

DWidgetDrag::~DWidgetDrag()
{
}

static const char* drag_formats[] = {
	"application/x-qbuilder-widget",
	0 };

const char * DWidgetDrag::format(int i) const
{
  if ( i >= 0 && i < 3 )
    return drag_formats[i];
  return 0;
}

QByteArray DWidgetDrag::encodedData(const char* mime) const
{
  QByteArray a;
  if ( strcmp( mime, "application/x-qbuilder-widget" ) == 0 )
  {
    QString d( "%1:%2:%3" );
    d = d.arg( data.sizeHint.width() ).arg( data.sizeHint.height() ).arg( data.className );
    int l = d.length();
    a.resize( l + 1 );
    memcpy( a.data(), d.ascii(), l );
    a[ l ] = 0;
  }

  return a;
}

bool DWidgetDrag::canDecode( QMimeSource* e )
{
    for ( int i=0; drag_formats[i]; i++ )
	if ( e->provides( drag_formats[i] ) )
	    return TRUE;
    return FALSE;
}

bool DWidgetDrag::decode( QMimeSource* e, DWidgetDrag::Widget& _info )
{
  QByteArray payload = e->encodedData( "application/x-qbuilder-widget" );
  if ( payload.size() )
  {
    QString tmp = payload.data();
    _info.sizeHint.setWidth( atoi( tmp.ascii() ) );
    int pos = tmp.find( ':' );
    if ( pos == -1 )
      return false;
    _info.sizeHint.setHeight( atoi( tmp.ascii() + pos + 1 ) );
    pos = tmp.find( ':', pos + 1 );
    if ( pos == -1 )
      return false;
    _info.className = tmp.mid( pos + 1 );
    return TRUE;
  }
  return FALSE;
}

/**********************************************
 *
 * DWidgetButton
 *
 **********************************************/

DWidgetButton::DWidgetButton( const DWidgetInfo& _info, QWidget* _parent, const char* _name )
  : QLabel( _parent, _name ), m_info( _info )
{
  m_pressed = false;

  setPixmap( m_info.pixmap() );
}

DWidgetButton::~DWidgetButton()
{
}

void DWidgetButton::mousePressEvent( QMouseEvent* _ev )
{
  if ( _ev->button() == Qt::LeftButton )
  {
    m_pressedPos = _ev->pos();
    m_pressed = true;
  }
}

void DWidgetButton::mouseMoveEvent( QMouseEvent* _ev )
{
  if ( !m_pressed )
    return;

  if ( abs( _ev->pos().x() - m_pressedPos.x() ) >= Dnd_X_Precision ||
       abs( _ev->pos().y() - m_pressedPos.y() ) >= Dnd_Y_Precision )
  {
      QPoint hotspot;
      hotspot.setX( pixmap()->width() / 2 );
      hotspot.setY( pixmap()->height() / 2 );

      DWidgetDrag* drag = new DWidgetDrag( m_info.className(), m_info.sizeHint(), this );
      drag->setPixmap( *pixmap(), hotspot );
      drag->drag();
  }
}

void DWidgetButton::mouseReleaseEvent( QMouseEvent* )
{
  m_pressed = false;
}

/**********************************************
 *
 * DWidgetsPage
 *
 **********************************************/

DWidgetsPage::DWidgetsPage( QWidget* _parent, const char* _name )
  : QFrame( _parent, _name )
{
  m_layout = new QHBoxLayout( this, 6, 6 );

  // Demo code
  QPixmap pixmap( "qpushbutton.xpm" );
  DWidgetInfo info( "QPushButton", pixmap, "A push button" );
  DWidgetButton* b = new DWidgetButton( info, this );
  m_layout->addWidget( b );

  QPixmap pixmap2( "qgroupbox.xpm" );
  DWidgetInfo info2( "QGroupBox", pixmap2, "A group box" );
  b = new DWidgetButton( info2, this );
  m_layout->addWidget( b );

  QPixmap pixmap3( "qtabwidget.xpm" );
  DWidgetInfo info3( "QTabWidget", pixmap3, "A tabbed widget" );
  b = new DWidgetButton( info3, this );
  m_layout->addWidget( b );

  QPixmap pixmap4( "qmultilineedit.xpm" );
  DWidgetInfo info4( "QMultiLineEdit", pixmap4, "An editor" );
  b = new DWidgetButton( info4, this );
  m_layout->addWidget( b );
    
  for( int i = 0; i < 5; i++ )
  {
    DWidgetInfo info( "QPushButton", pixmap, "A push button" );
    DWidgetButton* b = new DWidgetButton( info, this );
    m_layout->addWidget( b );
  }

  m_layout->addStretch( 1 );
}

DWidgetsPage::~DWidgetsPage()
{
}

/**********************************************
 *
 * DWidgetsBar
 *
 **********************************************/

DWidgetsBar::DWidgetsBar( QWidget* _parent, const char* _name )
  : QTabWidget( _parent, _name )
{
  // Demo Code
  addPage( "Buttons" );
  addPage( "Containers" );
  addPage( "Lists" );
  addPage( "Range Controls" );

  debug( "sizehint is %i %i", sizeHint().width(),sizeHint().height() );

  setMinimumSize( sizeHint() );
}

DWidgetsBar::~DWidgetsBar()
{
}

DWidgetsPage* DWidgetsBar::addPage( const QString& _label )
{
  DWidgetsPage* p = new DWidgetsPage( this );
  addTab( p, _label );

  debug( "child sizehint is %i %i", p->sizeHint().width(), p->sizeHint().height() );

  return p;
}

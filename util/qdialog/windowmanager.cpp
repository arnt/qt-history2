#include "windowmanager.h"

#include <qpainter.h>
#include <qpixmap.h>

QPixmap* DWindowManager::s_titleLeft = 0;
QPixmap* DWindowManager::s_titleRight = 0;

DWindowManager::DWindowManager( QWidget* parent, const char* name )
  : QFrame( parent, name )
{
  setBackgroundColor( QColor( "#cecece" ) );
  setFrameStyle( QFrame::Panel | QFrame::Raised );
  setLineWidth( 2 );

  if ( !s_titleLeft )
    // TODO: Find better name for file and care about the path
    s_titleLeft = new QPixmap( "title2.xpm" );
  if ( !s_titleRight )
    // TODO: Find better name for file and care about the path
    s_titleRight = new QPixmap( "title.xpm" );
}

DWindowManager::~DWindowManager()
{
}

void DWindowManager::paintEvent( QPaintEvent* _ev )
{
  QPainter painter;
  painter.begin( this );

  painter.drawPixmap( 2, 2, *s_titleLeft );
  painter.drawPixmap( width() - s_titleRight->width() - 2, 2, *s_titleRight );

  painter.end();

  QFrame::paintEvent( _ev );
}

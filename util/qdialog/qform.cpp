#include "qscrollbar.h"
#include "qsize.h"
#include "qevent.h"
#include "qapplication.h"

#include "qform.h"

QForm::QForm( QWidget* _parent = 0, const char* _name = 0 )
  : QFrame( _parent, _name )
{
  view = new QWidget( this );
  hbar = new QScrollBar( Horizontal, this );
  vbar = new QScrollBar( Vertical, this );
  
  view->setGeometry( 0, 0, width() - 20, height() - 20 );

  hbar->raise();
  vbar->raise();

  connect( hbar, SIGNAL( valueChanged( int ) ),
	   this, SLOT( scrollHor( int ) ) );
  connect( vbar, SIGNAL( valueChanged( int ) ),
	   this, SLOT( scrollVer( int ) ) );
}

QForm::~QForm()
{
}

bool QForm::event( QEvent *e )
{
  if ( e->type() != QEvent::LayoutHint )
    return QFrame::event( e );

  debug("---------- LayoutHint ----------" );
  
  // updateLayout();
  
  return FALSE;
}

void QForm::resizeEvent( QResizeEvent* )
{
  updateLayout();
}

void QForm::updateLayout()
{
  viewport()->resize( width() - 20, height() - 20 );
  
  QSize hint = view->sizeHint();
  
  bool vscroll = ( height() < hint.height() );
  bool hscroll = ( width() < hint.width() );
  if ( vscroll && !hscroll )
    hscroll = ( width() - 16 < hint.width() );
  if ( !vscroll && hscroll )
    vscroll = ( height() - 16 < hint.height() );
  
  if ( !vscroll && !hscroll )
  {
    vbar->hide();
    hbar->hide();
    view->setGeometry( 0, 0, width(), height() );
    return;
  }
  
  if ( vscroll )
  {    
    vbar->show();
    vbar->setGeometry( width() - 16, 0, 16, height() - ( hscroll ? 16 : 0 ) );
    vbar->setRange( 0, hint.height() - ( height() - ( hscroll ? 16 : 0 ) ) );
    if ( vbar->value() + height() - ( hscroll ? 16 : 0 ) > hint.height() )
      vbar->setValue( hint.height() - height() - ( hscroll ? 16 : 0 ) );
    vbar->raise();
  }
  else
    vbar->hide();
  
  if ( hscroll )
  {    
    hbar->show();
    hbar->setGeometry( 0, height() - 16, width() - ( vscroll ? 16 : 0 ), 16 );
    hbar->setRange( 0, hint.width() - ( width() - ( vscroll ? 16 : 0 ) ) );
    if ( hbar->value() + width() - ( vscroll ? 16 : 0 ) > hint.width() )
      hbar->setValue( hint.width() - width() - ( vscroll ? 16 : 0 ) );
    hbar->raise();
  }
  else
    hbar->hide();

  viewport()->resize( hscroll ? hint.width() : width() - 16,
		      vscroll ? hint.height() : height() - 16 );
}

void QForm::scrollVer( int _v )
{
  view->move( view->x(), -_v );
}

void QForm::scrollHor( int _v )
{
  view->move( -_v, view->y() );
}

void QForm::wheelEvent( QWheelEvent* e )
{
  debug("WHEEL");
  QApplication::sendEvent( vbar, e);
}

#include "dlayout.h"

DStretch::DStretch( QWidget* _parent, const QResource& _resource )
  : QWidget( _parent, _resource )
{
  m_orientation = Qt::Vertical;
  m_stretch = 1;
  setBackgroundColor( QColor( 0, 0, 0 ) );

  setMinimumSize( QSize( 6, 6 ) );
}

DStretch::DStretch( QWidget* _parent, const char* _name )
  : QWidget( _parent, _name )
{
  m_orientation = Qt::Vertical;
  m_stretch = 1;
  setBackgroundColor( QColor( 0, 0, 0 ) );

  setMinimumSize( QSize( 6, 6 ) );
}

DStretch::~DStretch()
{
}

QSizePolicy DStretch::sizePolicy() const
{
  return QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
}

QSize DStretch::sizeHint() const
{
  if ( m_orientation == Qt::Horizontal )
    return QSize( 6, 80 );
  else
    return QSize( 80, 6 );
}

void DStretch::setStretch( int _s )
{
  m_stretch = _s;
}

int DStretch::stretch()
{
  return m_stretch;
}

void DStretch::setOrientation( Qt::Orientation _o )
{
  m_orientation = _o;
}


DSpacing::DSpacing( QWidget* _parent, const QResource& _resource )
  : QWidget( _parent, _resource )
{
  setBackgroundColor( QColor( 50, 50, 50 ) );

  m_orientation = Qt::Vertical;
  setSpacing( 6 );
}

DSpacing::DSpacing( QWidget* _parent, const char* _name )
  : QWidget( _parent, _name )
{
  setBackgroundColor( QColor( 50, 50, 50 ) );

  m_orientation = Qt::Vertical;
  setSpacing( 6 );
}

DSpacing::~DSpacing()
{
}

QSizePolicy DSpacing::sizePolicy() const
{
  return QSizePolicy( m_orientation == Qt::Horizontal ? QSizePolicy::Fixed :
		                                        QSizePolicy::Expanding,
		      m_orientation == Qt::Vertical ? QSizePolicy::Fixed :
		                                      QSizePolicy::Expanding );
}

QSize DSpacing::sizeHint() const
{
  if ( m_orientation == Qt::Horizontal )
    return QSize( m_spacing, 80 );
  else
    return QSize( 80, m_spacing );
}

void DSpacing::setSpacing( int _s )
{
  m_spacing = _s;
  
  if ( m_orientation == Qt::Vertical )
  {
    setFixedHeight( QMAX( 6, _s ) );
    setMinimumWidth( 0 );
    setMaximumWidth( QCOORD_MAX );
  }
  else
  {
    setFixedWidth( QMAX( 6, _s ) );
    setMinimumHeight( 0 );
    setMaximumHeight( QCOORD_MAX );
  }
}

int DSpacing::spacing()
{
  return m_spacing;
}

void DSpacing::setOrientation( Qt::Orientation _o )
{
  m_orientation = _o;
  setSpacing( m_spacing );
}


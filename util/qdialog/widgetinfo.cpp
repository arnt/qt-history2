#include "widgetinfo.h"

#include <qressource.h>

DWidgetInfo::DWidgetInfo( const QString& _class, const QPixmap& _pixmap, const QString& _tooltip )
{
  m_className = _class;
  m_pixmap = _pixmap;
  m_toolTip = _tooltip;
}

QSize DWidgetInfo::sizeHint() const
{
  QWidget* w = QRessourceFactory::factory()->createWidget( m_className, 0 );
  ASSERT( w );
  QSize s = w->sizeHint();
  delete w;

  debug("Sizehint is %i %i", s.width(),s.height() );
  return s;
}

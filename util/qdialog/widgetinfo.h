#ifndef __dwidgetinfo_h__
#define __dwidgetinfo_h__

#include <qstring.h>
#include <qpixmap.h>

class DWidgetInfo
{
public:
  DWidgetInfo( const QString& _class, const QPixmap& _pixmap, const QString& _tooltip );

  QPixmap pixmap() const { return m_pixmap; }
  QString toolTip() const { return m_toolTip; }
  QString className() const { return m_className; }
  QSize sizeHint() const;

private:
  QString m_className;
  QString m_toolTip;
  QPixmap m_pixmap;
};

#endif

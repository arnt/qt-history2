#include "widgetinfo.h"

#include <qmetaobject.h>
#include <qwidget.h>
#include <qresource.h>

DWidgetInfo::DWidgetInfo( const QString& _class, const QPixmap& _pixmap, const QString& _tooltip )
{
  m_className = _class;
  m_pixmap = _pixmap;
  m_toolTip = _tooltip;
}

QSize DWidgetInfo::sizeHint() const
{
  QMetaObject* m = QMetaObjectInit::metaObject( m_className );
  ASSERT( m );
  QObjectFactory f = m->factory();
  ASSERT( f );
  QWidget* w = (QWidget*)(*f)( 0, QResource() );
  ASSERT( w != 0 );

  QSize s = w->sizeHint();
  delete w;

  qDebug("Sizehint is %i %i", s.width(),s.height() );
  return s;
}

QProperty DWidgetInfo::defaultProperty( const QString& classname, const QString& name )
{
  QMetaObject* m = QMetaObjectInit::metaObject( classname );
  ASSERT( m );
  QObjectFactory f = m->factory();
  ASSERT( f );
  QWidget* w = (QWidget*)(*f)( 0, QResource() );
  ASSERT( w != 0 );

  // Get a list of all properties
  QStringList props = m->propertyNames();
  
  QStringList::Iterator sit = props.begin();
  for( ; sit != props.end(); ++sit )
  { 
    QMetaProperty* p = m->property( *sit, TRUE );
    if ( p && !p->readonly )
    {
      if ( name == p->name )
      {
	QProperty prop;
	w->property( p->name, &prop );
	delete w;
	return prop;
      }
    }
  }

  delete w;

  return QProperty();
}

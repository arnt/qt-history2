#include "qwidget.h"
#include "qlayout.h"
#include "qmetaobject.h"
#include "qdom.h"
#include "qpixmap.h"

QWidget* QDomElement::toWidget( QWidget* _parent ) const
{
  // QWidget* w = new QWidget( _parent );

  if ( isNull() )
    return 0;

  if ( tagName() == "QDL" )
    return firstChild().toElement().toWidget( _parent );

  QMetaObject* m = QMetaObjectInit::metaObject( tagName() );
  if ( !m )
  {
    qDebug("Dont know class %s", tagName().ascii() );
    return 0;
  }

  if ( !m->inherits( "QWidget" ) )
  {
    qDebug("Class %s does not inherit from QWidget", tagName().ascii() );
    return 0;
  }

  QObjectFactory f = m->factory();
  if ( !f )
  {
    qDebug("Class %s has no factory", tagName().ascii() );
    return 0;
  }

  QObject* w = (*f)( _parent );
  if ( !w )
  {
    qDebug("Factory for class %s returned 0", tagName().ascii() );
    return 0;
  }

  /* if ( !w->setConfiguration( *this ) )
  {
    delete w;
    return 0;
  }
  */
  bool res = w->setConfiguration( *this );

  return (QWidget*)w;
}

QLayout* QDomElement::toLayout( QWidget* _parent ) const
{
  if ( isNull() )
    return 0;

  if ( _parent == 0 )
  {
    qDebug("0 passes to QDomElement::toLayout\n");
    return 0;
  }

  if ( tagName() == "QDL" )
    return firstChild().toElement().toLayout( _parent );

  QMetaObject* m = QMetaObjectInit::metaObject( tagName() );
  if ( !m )
  {
    qDebug("Dont know class %s", tagName().ascii() );
    return 0;
  }

  if ( !m->inherits( "QLayout" ) )
  {
    qDebug("Class %s does not inherit from QWidget", tagName().ascii() );
    return 0;
  }

  QObjectFactory f = m->factory();
  if ( !f )
  {
    qDebug("Class %s has no factory", tagName().ascii() );
    return 0;
  }

  QLayout* w = (QLayout*)(*f)( _parent );
  if ( !w )
  {
    qDebug("Factory for class %s returned 0", tagName().ascii() );
    return 0;
  }

  if ( ! w->setConfiguration( *this, _parent ) )
  {
    delete w;
    return 0;
  }

  return w;
}

QLayout* QDomElement::toLayout( QLayout* _parent, QWidget* mainwidget ) const
{
  if ( isNull() )
    return 0;

  QWidget* pw = mainwidget;
  if ( !pw )
    pw = mainwidget = _parent->mainWidget();
  if ( !pw )
  {
    qDebug("QDomElement::toLayout  Can not proceed without mainwidget\n");
    return 0;
  }

  if ( tagName() == "QDL" )
    return firstChild().toElement().toLayout( _parent );

  QMetaObject* m = QMetaObjectInit::metaObject( tagName() );
  if ( !m )
  {
    qDebug("Dont know class %s", tagName().ascii() );
    return 0;
  }

  if ( !m->inherits( "QLayout" ) )
  {
    qDebug("Class %s does not inherit from QWidget", tagName().ascii() );
    return 0;
  }

  QObjectFactory f = m->factory();
  if ( !f )
  {
    qDebug("Class %s has no factory", tagName().ascii() );
    return 0;
  }

  QLayout* w = 0;
  if ( !_parent )
    w = (QLayout*)(*f)( 0 );
  else
    w = (QLayout*)(*f)( _parent );
  if ( !w )
  {
    qDebug("Factory for class %s returned 0", tagName().ascii() );
    return 0;
  }

  if ( ! w->setConfiguration( *this, pw ) )
  {
    delete w;
    return 0;
  }

  return w;
}

QVariant QDomElement::property( const QString& name, QVariant::Type type ) const
{
  switch( type )
  {
  case QVariant::String:
    {
      if ( name == "name" )
	return QVariant( attribute( name ) );

      QDomElement n = namedItem( name ).toElement();
      if ( !n.isNull() )
	return QVariant( n.text() );
    }
    break;
  case QVariant::Bool:
    {
      if ( hasAttribute( name ) )
	return QVariant( (bool)attribute( name ).toInt() );
    }
    break;
  case QVariant::Int:
    {
      if ( hasAttribute( name ) )
	return QVariant( attribute( name ).toInt() );
    }
    break;
  case QVariant::Double:
    {
      if ( hasAttribute( name ) )
	return QVariant( attribute( name ).toDouble() );
    }
    break;
  case QVariant::Color:
    {
      if ( hasAttribute( name ) )
	return QVariant( QColor( attribute( name ) ) );
    }
    break;
  case QVariant::Font:
    {
      QDomElement n = namedItem( name ).toElement();
      if ( !n.isNull() )
	return QVariant( n.toFont() );
    }
    break;
  case QVariant::Rect:
    {
      QDomElement n = namedItem( name ).toElement();
      if ( !n.isNull() )
	return QVariant( n.toRect() );
    }
    break;
  case QVariant::Size:
    {
      QDomElement n = namedItem( name ).toElement();
      if ( !n.isNull() )
	return QVariant( n.toSize() );
    }
    break;
  case QVariant::Point:
    {
      QDomElement n = namedItem( name ).toElement();
      if ( !n.isNull() )
	return QVariant( n.toPoint() );
    }
    break;
  case QVariant::StringList:
    {
      QDomElement n = namedItem( name ).toElement();
      if ( n.isNull() )
	return FALSE;

      QStringList lst;
      QDomElement e = n.firstChild().toElement();
      for( ; !e.isNull(); e = e.nextSibling().toElement() )
	if ( e.tagName() == "StringItem" )
	  lst.append( e.text() );

      return QVariant( lst );
    }
    break;
  case QVariant::IntList:
    {
      QDomElement n = namedItem( name ).toElement();
      if ( n.isNull() )
	return FALSE;

      QValueList<int> lst;
      QDomElement e = n.firstChild().toElement();
      for( ; !e.isNull(); e = e.nextSibling().toElement() )
	if ( e.tagName() == "IntItem" )
	  lst.append( e.attribute( "value" ).toInt() );

      return QVariant( lst );
    }
    break;
  case QVariant::DoubleList:
    {
      QDomElement n = namedItem( name ).toElement();
      if ( n.isNull() )
	return FALSE;

      QValueList<double> lst;
      QDomElement e = n.firstChild().toElement();
      for( ; !e.isNull(); e = e.nextSibling().toElement() )
	if ( e.tagName() == "DoubleItem" )
	  lst.append( e.attribute( "value" ).toInt() );

      return QVariant( lst );
      }
    break;
  case QVariant::Pixmap:
    {
      if ( hasAttribute( name ) )
      {
	QDomMimeSourceFactory* f = ownerDocument().mimeSourceFactory();
	return QVariant( f->pixmap( attribute( name ) ) );
	/* const QMimeSource* m = f->data( attribute( name ) );
	if ( m && m->provides( "image/x-xpm" ) )
	  return QVariant( QPixmap( m->encodedData( "image/x-xpm" ) ) );
	if ( m && m->provides( "image/png" ) )
	  return QVariant( QPixmap( m->encodedData( "image/png" ) ) );
	if ( m && m->provides( "image/jpeg" ) )
	  return QVariant( QPixmap( m->encodedData( "image/jpeg" ) ) ); */
      }
    }
    break;
  case QVariant::Brush:
  case QVariant::Palette:
  case QVariant::ColorGroup:
  case QVariant::Image:
    //##### TODO
    break;
  case QVariant::Custom:
  case QVariant::NTypes:
  case QVariant::Empty:
    // Do nothing
    break;
  }

  return QVariant();
}

void QDomElement::setProperty( const QString& name, const QVariant& prop )
{
  if ( prop.isEmpty() )
    return;

  QDomDocument doc = ownerDocument();

  switch( prop.type() )
  {
  case QVariant::String:
    {
      if ( prop.stringValue().isEmpty() )
	return;

      if ( name == "name" )
	setAttribute( "name", prop.stringValue() );
      else
      {
	QDomElement item = doc.createElement( name );
	item.appendChild( doc.createTextNode( prop.stringValue() ) );
	appendChild( item );
      }
    }
    break;
  case QVariant::StringList:
    {
      QDomElement item = doc.createElement( name );
      item.appendChild( item );

      QStringList::ConstIterator it = prop.stringListValue().begin();
      QStringList::ConstIterator end = prop.stringListValue().end();
      for( ; it != end; ++it )
      {
	QDomElement li = doc.createElement( "StringItem" );
	item.appendChild( li );
	QDomText text = doc.createTextNode( *it );
	li.appendChild( text );
      }
    }
    break;
  case QVariant::IntList:
    {
      QDomElement item = doc.createElement( name );
      appendChild( item );

      QValueList<int>::ConstIterator it = prop.intListValue().begin();
      QValueList<int>::ConstIterator end = prop.intListValue().end();
      for( ; it != end; ++it )
      {
	QDomElement li = doc.createElement( "IntItem" );
	QString str;
	str.setNum( *it );
	li.setAttribute( "value", str );
	item.appendChild( li );
      }
    }
    break;
  case QVariant::DoubleList:
    {
      QDomElement item = doc.createElement( name );
      appendChild( item );

      QValueList<double>::ConstIterator it = prop.doubleListValue().begin();
      QValueList<double>::ConstIterator end = prop.doubleListValue().end();
      for( ; it != end; ++it )
      {
	QDomElement li = doc.createElement( "DoubleItem" );
	QString str;
	str.setNum( *it );
	li.setAttribute( "value", str );
	item.appendChild( li );
      }
    }
    break;
  case QVariant::Font:
    {
      QDomElement item = doc.createElement( name, prop.fontValue() );
      appendChild( item );
    }
    break;
    // MovieType,
  case QVariant::Pixmap:
    {
	QString n = ownerDocument().mimeSourceFactory()->pixmapName( prop.pixmapValue() );
	ASSERT( !n.isEmpty() );
	setAttribute( name, n );
    }
    break;
  case QVariant::Brush:
    //##### TODO
    break;
  case QVariant::Rect:
    {
      QDomElement item = doc.createElement( name, prop.rectValue() );
      appendChild( item );
    }
    break;
  case QVariant::Size:
    {
      QDomElement item = doc.createElement( name, prop.sizeValue() );
      appendChild( item );
    }
    break;
  case QVariant::Color:
    {
      setAttribute( name, prop.colorValue().name() );
    }
    break;
  case QVariant::Palette:
    //##### TODO
    break;
  case QVariant::ColorGroup:
    //##### TODO
    break;
  case QVariant::Point:
    {
      QDomElement item = doc.createElement( name, prop.pointValue() );
      appendChild( item );
    }
    break;
  case QVariant::Image:
    // ##### TODO
    break;
  case QVariant::Int:
    {
      QString str;
      str.setNum( prop.intValue() );
      setAttribute( name, str );
    }
    break;
  case QVariant::Bool:
    if ( prop.boolValue() )
      setAttribute( name, "1" );
    else
      setAttribute( name, "0" );
    break;
  case QVariant::Double:
    {
      QString str;
      str.setNum( prop.doubleValue() );
      setAttribute( name, str );
    }
    break;
  case QVariant::Custom:
    //###### TODO
    break;
  case QVariant::Empty:
  case QVariant::NTypes:
    // Do nothing
    break;
  }
}

#include "xml.h"
#include "formeditor.h"

#include <qobject.h>
#include <qxmlparser.h>
#include <qproperty.h>
#include <qfont.h>
#include <qrect.h>
#include <qsize.h>
#include <qpoint.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qmetaobject.h>

void qPropertyToXML( QXMLTag* tag, const QProperty& prop, const QString& _name )
{
  if ( prop.isEmpty() )
    return;

  switch( prop.type() )
    {
    case QProperty::StringType:
      {
	if ( prop.stringValue().isEmpty() )
	  return;

	if ( _name == "name" )
	  tag->insertAttrib( "name", prop.stringValue() );
	else
	{
	  QXMLTag* t = new QXMLTag( _name );
	  t->insert( new QXMLTag( prop.stringValue(), TRUE ) );
	  tag->insert( tag->end(), t );
	}
      }
      break;
    case QProperty::StringListType:
      break;
    case QProperty::IntListType:
      break;
    case QProperty::DoubleListType:
      break;
    case QProperty::FontType:
      {
	QXMLTag* t = new QXMLTag( _name );
	tag->insert( tag->end(), t );
	QXMLTag* f = new QXMLTag( "QFont" );
	t->insert( f );
	f->insertAttrib( "family", prop.fontValue().family() );

	QString str;
	str.setNum( prop.fontValue().pointSize() );
	f->insertAttrib( "size", str );

	switch( prop.fontValue().weight() )
	  {
	  case QFont::Light:
	    str = "light";
	    break;
	  case QFont::Normal:
	    str = "normal";
	    break;
	  case QFont::DemiBold:
	    str = "demibold";
	    break;
	  case QFont::Black:
	    str = "black";
	    break;
	  case QFont::Bold:
	    str = "bold";
	    break;
	  }
	f->insertAttrib( "weight", str );

	if ( prop.fontValue().underline() )
	  f->insertAttrib( "underline", "true" );
	if ( prop.fontValue().italic() )
	  f->insertAttrib( "italic", "true" );
      }
      break;
      // MovieType,
    case QProperty::PixmapType:
      // TODO
      break;
    case QProperty::BrushType:
      // TODO
      break;
    case QProperty::RectType:
      {
	QXMLTag* t = new QXMLTag( _name );
	tag->insert( tag->end(), t );
	QXMLTag* f = new QXMLTag( "QRect" );
	t->insert( f );
	QString str;
	str.setNum( prop.rectValue().x() );
	f->insertAttrib( "x", str );
	str.setNum( prop.rectValue().y() );
	f->insertAttrib( "y", str );
	str.setNum( prop.rectValue().width() );
	f->insertAttrib( "width", str );
	str.setNum( prop.rectValue().height() );
	f->insertAttrib( "height", str );
      }
      break;
    case QProperty::SizeType:
      {
	QXMLTag* t = new QXMLTag( _name );
	tag->insert( tag->end(), t );
	QXMLTag* f = new QXMLTag( "QSize" );
	t->insert( f );
	QString str;
	str.setNum( prop.sizeValue().width() );
	f->insertAttrib( "width", str );
	str.setNum( prop.sizeValue().height() );
	f->insertAttrib( "height", str );
      }
      break;
    case QProperty::ColorType:
      {
	QString str( "#%1%2%3" );
	str = str.arg( prop.colorValue().red(), 2, 16 ).arg( prop.colorValue().green(), 2, 16 ).arg( prop.colorValue().blue(), 2, 16 );
	tag->insertAttrib( _name, str );
      }
      break;
    case QProperty::PaletteType:
      // TODO
      break;
    case QProperty::ColorGroupType:
      // TODO
      break;
    case QProperty::PointType:
      {
	QXMLTag* t = new QXMLTag( _name );
	tag->insert( tag->end(), t );
	QXMLTag* f = new QXMLTag( "QPoint" );
	t->insert( f );
	QString str;
	str.setNum( prop.pointValue().x() );
	f->insertAttrib( "x", str );
	str.setNum( prop.pointValue().y() );
	f->insertAttrib( "y", str );
      }
      break;
    case QProperty::ImageType:
      // TODO
      break;
    case QProperty::IntType:
      {
	QString str;
	str.setNum( prop.intValue() );
	tag->insertAttrib( _name, str );
      }
      break;
    case QProperty::BoolType:
      if ( prop.boolValue() )
	tag->insertAttrib( _name, "true" );
      else
	tag->insertAttrib( _name, "false" );
      break;
    case QProperty::DoubleType:
      {
	QString str;
	str.setNum( prop.doubleValue() );
	tag->insertAttrib( _name, str );
      }
      break;
    case QProperty::CustomType:
    case QProperty::Empty:
      // Do nothing
      break;
    }
}

QXMLTag* qObjectToXML( QObject* o, bool _layouted )
{
  QXMLTag* t = new QXMLTag( o->className() );

  QMetaObject* m = o->metaObject();
  if ( m )
  {
    QStringList props = m->propertyNames();
  
    QStringList::Iterator it = props.begin();
    for( ; it != props.end(); ++it )
    {
      QMetaProperty* p = m->property( *it, TRUE );
      if ( p && !p->readonly )
      {
	// Dont save absolute positions in this case
	if ( !_layouted || strcmp( p->name, "geometry" ) != 0 )
	{
	  QProperty prop;
	  o->property( p->name, &prop );
	  qPropertyToXML( t, prop, p->name );
	}
      }
    }
  }

  // TODO embedded DFormWidgets

  return t;
}

QXMLTag* qObjectToXML( DObjectInfo* o, bool _layouted )
{
  QXMLTag *t = qObjectToXML( o->widget(), _layouted );

  QValueList<DObjectInfo::Connection>::Iterator it = o->connectionsBegin();
  QValueList<DObjectInfo::Connection>::Iterator end = o->connectionsEnd();
  for( ; it != end; ++it )
  {
    QXMLTag* c = new QXMLTag( "Connection" );
    c->insertAttrib( "sender", it->senderName );
    c->insertAttrib( "signal", it->signal );
    c->insertAttrib( "receiver", it->receiverName );
    if ( it->slotIsSignal )
      c->insertAttrib( "transmitter", it->slot );
    else
      c->insertAttrib( "slot", it->slot );
    t->insert( c );
  }

  QStringList::ConstIterator sit = o->customSignals().begin();
  QStringList::ConstIterator send = o->customSignals().end();
  for( ; sit != send; ++sit )
  {
    QXMLTag* c = new QXMLTag( "CustomSignal" );
    c->insertAttrib( "signature", *sit );
    t->insert( c );
  }

  sit = o->customSlots().begin();
  send = o->customSlots().end();
  for( ; sit != send; ++sit )
  {
    QXMLTag* c = new QXMLTag( "CustomSlot" );
    c->insertAttrib( "signature", *sit );
    t->insert( c );
  }

  return t;
}

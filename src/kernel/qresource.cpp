/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qresource.cpp#7 $
**
** Implementation of QResource classes
**
** Created : 990128
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qresource.h"
#include "qxml.h"
#include "qfile.h"
#include "qtextstream.h"
#include "qmetaobject.h"
#include "qfont.h"
#include "qwidget.h"
#include "qlayout.h"

// NOT REVISED

/*********************************************
 *
 * QResourceXMLConsumer definition
 *
 *********************************************/

class QResourceXMLConsumer : public QXMLConsumer
{
public:
  QResourceXMLConsumer();
  ~QResourceXMLConsumer();

  virtual bool tagStart( const QString& name );
  virtual bool tagEnd( const QString& name );
  virtual bool attrib( const QString& name, const QString& value );
  virtual bool text( const QString& text );
  virtual bool entity( const QString& name, const QString& value );
  virtual void parseError( int pos );
  virtual bool finished();

  QResourceItem* donateTree() { QResourceItem* i = item; item = 0; return i; }

private:
  QResourceItem *item;
};

/*********************************************
 *
 * QResourceItem
 *
 *********************************************/

QResourceItem::QResourceItem( const QString& _type, bool istext )
{
  bIsText = istext;
  firstChildItem = nextSiblingItem = parentItem = 0;
  txt = _type;
}

QResourceItem* QResourceItem::child( const QString& type )
{
  QResourceItem* item = firstChildItem;
  for( ; item; item = item->nextSibling() )
    if ( item->type() == type )
      return item;

  return 0;
}

const QResourceItem* QResourceItem::child( const QString& type ) const
{
  const QResourceItem* item = firstChildItem;
  for( ; item; item = item->nextSibling() )
    if ( item->type() == type )
      return item;

  return 0;
}

void QResourceItem::clear()
{
  parentItem = 0;
  if ( firstChildItem )
    delete firstChildItem;
  firstChildItem = nextSiblingItem = 0;
  attribs.clear();
  bIsText = FALSE;
  txt = QString::null;
}

void QResourceItem::prepend( QResourceItem* _tag )
{
  _tag->nextSiblingItem = firstChildItem;
  firstChildItem = _tag;
  _tag->parentItem = this;
}
	
void QResourceItem::insert( const QResourceItem* pos, QResourceItem* _tag )
{
  if ( !_tag  )
	return;

  if ( pos == firstChildItem )
  {
    _tag->nextSiblingItem = firstChildItem;
    firstChildItem = _tag;
  }
  else
  {
     QResourceItem* t = firstChildItem;
     while ( t && pos != t->nextSiblingItem )
	t = t->nextSiblingItem;
     if ( !t )
     {
	qWarning("Illegal Iterator in QResourceItem::insert");
	return;
     }
     _tag->nextSiblingItem = t->nextSiblingItem;
     t->nextSiblingItem = _tag;
  }

  _tag->parentItem = this;
}

void QResourceItem::replace( const QResourceItem* pos, QResourceItem* neu )
{
  QResourceItem* t = extractAndReplace( pos, neu );
  if ( t )
    delete t;
}

QResourceItem* QResourceItem::extractAndReplace( const QResourceItem* pos, QResourceItem* neu )
{
  if ( pos == end() )
  {
    qDebug("QResourceItem::extractAndReplace()  Can not replace with invalid position");
    return 0;
  }

  if ( pos == firstChildItem )
  {
    printf("Extraxt first\n");
    QResourceItem* t = firstChildItem;
    firstChildItem = neu;
    neu->parentItem = this;
    neu->nextSiblingItem = t->nextSiblingItem;
    t->nextSiblingItem = 0;
    t->parentItem = 0;
    return t;
  }

  QResourceItem* t = firstChildItem;
  while ( t && pos != t->nextSiblingItem )
     t = t->nextSiblingItem;

  if ( t == 0 )
  {
    qWarning("QResourceItem::extractAndReplace  Illegal Iterator");
    return 0;
  }

  QResourceItem* res = t->nextSiblingItem;
  t->nextSiblingItem = neu;
  neu->nextSiblingItem = res->nextSiblingItem;
  neu->parentItem = this;
  res->nextSiblingItem = 0;
  res->parentItem = 0;

  return res;
}

QResourceItem* QResourceItem::extract( const QResourceItem* pos )
{
  if ( pos == end() )
    return 0;

  if ( pos == firstChildItem )
  {
    QResourceItem* t = firstChildItem;
    firstChildItem = firstChildItem->nextSiblingItem;
    t->nextSiblingItem = 0;
    t->parentItem = 0;
    return t;
  }

  QResourceItem* t = firstChildItem;
  while ( t && pos != t->nextSiblingItem )
     t = t->nextSiblingItem;

  if ( t == 0 )
  {
    qWarning("Illegal Iterator in QResourceItem::insert");
    return 0;
  }

  QResourceItem* res = t->nextSiblingItem;
  t->nextSiblingItem = t->nextSiblingItem->nextSiblingItem;

  res->nextSiblingItem = 0;
  res->parentItem = 0;

  return res;
}

uint QResourceItem::childCount() const
{
  uint c = 0;
  QResourceItem* t = firstChildItem;
  while( t )
  {
    ++c;
    t = t->nextSiblingItem;
  }

  return c;
}

int QResourceItem::intAttrib( const QString& _name ) const
{
  return attribs[ _name ].toInt();
}

double QResourceItem::doubleAttrib( const QString& _name ) const
{
  return attribs[ _name ].toDouble();
}

bool QResourceItem::boolAttrib( const QString& _name ) const
{
  if ( strcasecmp( attribs[ _name ], "true" ) == 0 )
    return true;
  return false;
}

QColor QResourceItem::colorAttrib( const QString& _name ) const
{
  return QColor( attribs[ _name ] );
}

bool QResourceItem::hasAttrib( const QString& _name ) const
{
  return ( attribs.contains( _name ) );
}

bool QResourceItem::stringProperty( const QString& _name, QString* _val ) const
{
    // Special handling for the "name" property
    if ( _name == "name" )
    {
      QString n = name();
      if ( n.isEmpty() )
	return FALSE;
      *_val = n;
      return TRUE;
    }

    const QResourceItem* item = child( _name );
    if ( !item )
      return FALSE;

    *_val = "";
    const QResourceItem* i = item->firstChild();
    for( ; i; i = i->nextSibling() )
      *_val += i->text();

    return TRUE;
}

QProperty QResourceItem::property( const QMetaProperty* _p ) const
{
  return property( _p->name, QProperty::nameToType( _p->type ) );
}

QProperty QResourceItem::property( const QString& name, QProperty::Type type ) const
{
  switch( type )
    {
    case QProperty::StringType:
      {
	QString str;
	if ( !stringProperty( name, &str ) )
	  return QProperty();
	return QProperty( str );
      }
      break;
    case QProperty::BoolType:
      {
	if ( hasAttrib( name ) )
	  return QProperty( boolAttrib( name ) );
	return QProperty();
      }
      break;
    case QProperty::IntType:
      {
	if ( hasAttrib( name ) )
	  return QProperty( intAttrib( name ) );
	return QProperty();
      }
      break;
    case QProperty::DoubleType:
      {
	if ( hasAttrib( name ) )
	  return QProperty( doubleAttrib( name ) );
	return QProperty();
      }
      break;
    case QProperty::ColorType:
      {
	if ( hasAttrib( name ) )
	  return QProperty( colorAttrib( name ) );
	return QProperty();
      }
      break;
    case QProperty::FontType:
      {
	const QResourceItem* item = child( name );
	if ( !item )
	  return QProperty();
	
	item = item->child( "QFont" );
	if ( !item )
	  return QProperty();

	QFont font;
	if ( item->hasAttrib( "family" ) )
	  font.setFamily( item->attrib( "family" ) );
	if ( item->hasAttrib( "size" ) )
	  font.setPointSize( item->intAttrib( "size" ) );
	if ( item->hasAttrib( "weight" ) )
	{
	  QString w = item->attrib( "weight" );
	  if ( w == "light" )
	    font.setWeight( QFont::Light );
	  else if ( w == "normal" )
	    font.setWeight( QFont::Normal );
	  else if ( w == "demibold" )
	    font.setWeight( QFont::DemiBold );
	  else if ( w == "bold" )
	    font.setWeight( QFont::Bold );
	  else if ( w == "black" )
	    font.setWeight( QFont::Black );
	}
	if ( item->hasAttrib( "italic" ) && item->attrib( "italic" ) == "true" )
	  font.setItalic( TRUE );
	if ( item->hasAttrib( "underline" ) && item->attrib( "underline" ) == "true" )
	  font.setUnderline( TRUE );

	return QProperty( font );
      }
      break;
    case QProperty::RectType:
      {
	const QResourceItem* item = child( name );
	if ( !item )
	  return QProperty();
	
	item = item->child( "QRect" );
	if ( !item )
	  return QProperty();

	if ( item->hasAttrib( "x" ) && item->hasAttrib( "y" ) &&
	     item->hasAttrib( "width" ) && item->hasAttrib( "height" ) )
	{
	  QRect rect( item->intAttrib( "x" ), item->intAttrib( "y" ),
		      item->intAttrib( "width" ), item->intAttrib( "height" ) );
	  return QProperty( rect );
	}
	return QProperty();
      }
      break;
    case QProperty::SizeType:
      {
	const QResourceItem* item = child( name );
	if ( !item )
	  return QProperty();

	item = item->child( "QSize" );
	if ( !item )
	  return QProperty();
	
	if ( item->hasAttrib( "width" ) && item->hasAttrib( "height" ) )
	{
	  QSize size( item->intAttrib( "width" ), item->intAttrib( "height" ) );
	  return QProperty( size );
	}
	return QProperty();
      }
    case QProperty::PointType:
      {
	const QResourceItem* item = child( name );
	if ( !item )
	  return QProperty();
	
	item = item->child( "QPoint" );
	if ( !item )
	  return QProperty();

	if ( item->hasAttrib( "x" ) && item->hasAttrib( "y" ) )
	{
	  QPoint point( item->intAttrib( "x" ), item->intAttrib( "y" ) );
	  return QProperty( point );
	}
	return QProperty();
      }
    case QProperty::StringListType:
      {
	const QResourceItem* item = child( name );
	if ( !item )
	  return QProperty();

	item = item->child( "QStringList" );
	if ( !item )
	  return QProperty();

	QStringList lst;
	
	item = item->firstChild();
	for( ; item; item = item->nextSibling() )
	{
	  if ( item->type() == "li" )
	  {
	    QString str = "";
	    const QResourceItem* t = item->firstChild();
	    for( ; t; t = t->nextSibling() )
	      str =+ t->text();
	    lst.append( str );
	  }
	}
	
	return QProperty( lst );
      }
    case QProperty::IntListType:
      {
	const QResourceItem* item = child( name );
	if ( !item )
	  return QProperty();
	
	item = item->child( "QValueList<int>" );
	if ( !item )
	  return QProperty();

	QValueList<int> lst;

	item = item->firstChild();
	for( ; item; item = item->nextSibling() )
	  if ( item->type() == "li" && item->hasAttrib( "value" ) )
	    lst.append( item->intAttrib( "value" ) );

	return QProperty( lst );
      }
    case QProperty::DoubleListType:
      {
	const QResourceItem* item = child( name );
	if ( !item )
	  return QProperty();
	
	item = item->child( "QValueList<double>" );
	if ( !item )
	  return QProperty();

	QValueList<double> lst;

	item = item->firstChild();
	for( ; item; item = item->nextSibling() )
	  if ( item->type() == "li" && item->hasAttrib( "value" ) )
	    lst.append( item->doubleAttrib( "value" ) );
	
	return QProperty( lst );
      }
    case QProperty::CustomType:
    case QProperty::PixmapType:
    case QProperty::BrushType:
    case QProperty::PaletteType:
    case QProperty::ColorGroupType:
    case QProperty::ImageType:
      //##### TODO
      break;
    case QProperty::NTypes:
    case QProperty::Empty:
      // Do nothing
      break;
    }

  return QProperty();
}

void QResourceItem::setProperty( const QString& name, const QProperty& prop )
{
  if ( prop.isEmpty() )
    return;

  switch( prop.type() )
    {
    case QProperty::StringType:
      {
	if ( prop.stringValue().isEmpty() )
	  return;

	if ( name == "name" )
	  insertAttrib( "name", prop.stringValue() );
	else
	{
	  QResourceItem* item = new QResourceItem( name );
	  item->append( new QResourceItem( prop.stringValue(), TRUE ) );
	  append( item );
	}
      }
      break;
    case QProperty::StringListType:
      {
	QResourceItem* item = new QResourceItem( name );
	item->append( item );

	QResourceItem* lst = new QResourceItem( "QStringList" );
	append( lst );
	// Iterate backwards since we insert at the front
	QStringList::ConstIterator it = prop.stringListValue().fromLast();
	QStringList::ConstIterator end = prop.stringListValue().end();
	for( ; it != end; --it )
	{
	  QResourceItem* li = new QResourceItem( "li" );
	  lst->append( li );
	  QResourceItem* text = new QResourceItem( *it, TRUE );
	  li->append( text );
	}
      }
      break;
    case QProperty::IntListType:
      {
	QResourceItem* item = new QResourceItem( name );
	append( item );

	QResourceItem* lst = new QResourceItem( "QValueList<int>" );
	item->append( lst );
	// Iterate backwards since we insert at the front
	QValueList<int>::ConstIterator it = prop.intListValue().fromLast();
	QValueList<int>::ConstIterator end = prop.intListValue().end();
	for( ; it != end; --it )
	{
	  QResourceItem* li = new QResourceItem( "li" );
	  QString str;
	  str.setNum( *it );
	  li->insertAttrib( "value", str );
	  lst->append( li );
	}
      }
      break;
    case QProperty::DoubleListType:
      {
	QResourceItem* item = new QResourceItem( name );
	append( item );

	QResourceItem* lst = new QResourceItem( "QValueList<double>" );
	item->append( lst );
	// Iterate backwards since we insert at the front
	QValueList<double>::ConstIterator it = prop.doubleListValue().fromLast();
	QValueList<double>::ConstIterator end = prop.doubleListValue().end();
	for( ; it != end; --it )
	{
	  QResourceItem* li = new QResourceItem( "li" );
	  QString str;
	  str.setNum( *it );
	  li->insertAttrib( "value", str );
	  lst->append( li );
	}
      }
      break;
    case QProperty::FontType:
      {
	QResourceItem* item = new QResourceItem( name );
	append( item );

	QResourceItem* f = new QResourceItem( "QFont" );
	f->insertAttrib( "family", prop.fontValue().family() );
	item->append( f );

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
      //#####  TODO
      break;
    case QProperty::BrushType:
      //##### TODO
      break;
    case QProperty::RectType:
      {
	QResourceItem* item = new QResourceItem( name );
	append( item );

	QResourceItem* f = new QResourceItem( "QRect" );
	item->append( f );

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
	QResourceItem* item = new QResourceItem( name );
	append( item );

	QResourceItem* f = new QResourceItem( "QSize" );
	item->append( f );

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
	insertAttrib( name, str );
      }
      break;
    case QProperty::PaletteType:
      //##### TODO
      break;
    case QProperty::ColorGroupType:
      //##### TODO
      break;
    case QProperty::PointType:
      {
	QResourceItem* item = new QResourceItem( name );
	append( item );

	QResourceItem* f = new QResourceItem( "QPoint" );
	item->append( f );

	QString str;
	str.setNum( prop.pointValue().x() );
	f->insertAttrib( "x", str );
	str.setNum( prop.pointValue().y() );
	f->insertAttrib( "y", str );
      }
      break;
    case QProperty::ImageType:
      // ##### TODO
      break;
    case QProperty::IntType:
      {
	QString str;
	str.setNum( prop.intValue() );
	insertAttrib( name, str );
      }
      break;
    case QProperty::BoolType:
      if ( prop.boolValue() )
	insertAttrib( name, "true" );
      else
	insertAttrib( name, "false" );
      break;
    case QProperty::DoubleType:
      {
	QString str;
	str.setNum( prop.doubleValue() );
	insertAttrib( name, str );
      }
      break;
    case QProperty::CustomType:
      //###### TODO
      break;
    case QProperty::Empty:
    case QProperty::NTypes:
      // Do nothing
      break;
    }
}

/*********************************************
 *
 * QResourceIterator
 *
 *********************************************/

QResourceIterator QResourceIterator::operator++ (int)
{
  /* if ( !tag )
    return *this;

  QResourceItem* tmp = *this;
  tag = tag->nextSiblingItem;
  return tmp; */
}

QResourceIterator& QResourceIterator::operator++ ()
{
  /* if ( tag )
    tag = tag->nextSiblingItem;
    return *this; */
}

/*********************************************
 *
 * QResourceConstIterator
 *
 *********************************************/

QResourceConstIterator QResourceConstIterator::operator++ (int)
{
  /* if ( !tag )
    return *this;

  const QResourceItem* tmp = *this;
  tag = tag->nextSiblingItem;
  return tmp; */
}

QResourceConstIterator& const QResourceConstIterator::operator++ ()
{
  /* if ( tag )
    tag = tag->nextSiblingItem;
    return *this; */
}

/*********************************************
 *
 * QResourceXMLConsumer
 *
 *********************************************/

QResourceXMLConsumer::QResourceXMLConsumer()
{
  item = 0;
}

QResourceXMLConsumer::~QResourceXMLConsumer()
{
  if ( item )
    delete item;
}

bool QResourceXMLConsumer::tagStart( const QString& name )
{
  qDebug("Start=%s\n",name.ascii() );

  if ( !item )
    item = new QResourceItem( name );
  else
  {
    QResourceItem* i = new QResourceItem( name );
    item->append( i );
    item = i;
  }

  return TRUE;
}

bool QResourceXMLConsumer::tagEnd( const QString& name )
{
  qDebug("End=%s\n",name.ascii());
  if ( !item )
    return FALSE;

  if ( item->type() != name )
  {
    qDebug("Tag %s does not close %s\n",item->type().ascii(),name.ascii() );
    return FALSE;
  }

  if ( item->parent() )
    item = item->parent();

  return TRUE;
}

bool QResourceXMLConsumer::attrib( const QString& name, const QString& value )
{
  if ( !item )
    return FALSE;

  item->insertAttrib( name, value );

  return TRUE;
}

bool QResourceXMLConsumer::text( const QString& text )
{
  if ( !item )
    return FALSE;

  item->append( new QResourceItem( text, TRUE ) );

  return TRUE;
}

bool QResourceXMLConsumer::entity( const QString& name, const QString& value )
{
  if ( !value.isEmpty() )
    text( value );

  return TRUE;
}

void QResourceXMLConsumer::parseError( int )
{
  if ( item )
  {
    delete item;
    item = 0;
  }
}

bool QResourceXMLConsumer::finished()
{
  return TRUE;
}

/*********************************************
 *
 * QResource Streaming
 *
 *********************************************/

static void qWriteResourceItem( QTextStream& text, const QResourceItem& item, int indent )
{
  // Write indentation
  for( int i = 0; i < indent; ++i )
    text << " ";

  if ( item.isText() )
  {
    text << item.text() << endl;
    return;
  }

  text << "<" << item.type();

  QMap<QString,QString>::ConstIterator it = item.attribsMap().begin();
  QMap<QString,QString>::ConstIterator e = item.attribsMap().end();
  for( ; it != e; ++it )
    text << " " << it.key() << "=\"" << it.data() << "\"";

  // Does it have children ?
  if ( item.firstChild() )
  {
    text << ">" << endl;

    const QResourceItem* it = item.firstChild();
    while( it )
    {
      qWriteResourceItem( text, *it, indent + 1 );
      it = it->nextSibling();
    }

    // Write indentation
    for( int i = 0; i < indent; ++i )
      text << " ";

    text << "</" << item.type() << ">" << endl;
  }
  else
    text << "/>" << endl;
}

QTextStream& operator<< ( QTextStream& text, const QResourceItem& item )
{
  qWriteResourceItem( text, item, 0 );

  return text;
}

QTextStream& operator<< ( QTextStream& text, const QResource& resource )
{
  if ( resource.isEmpty() )
    return text;

  const QResourceItem* it = resource.tree();
  if ( it )
    text << *it;

  return text;
}

QTextStream& operator>> ( QTextStream& text, QResource& resource )
{
  resource.setContent( text.read() );

  return text;
}


/*********************************************
 *
 * QResource
 *
 *********************************************/

QResource::QResource()
{
  d = 0;
  owns = FALSE;
}

/*!
 By default the resource does NOT own the QResourceItem. That means that
 it wont delete it in its destructor.
*/
QResource::QResource( QResourceItem* item, bool own )
{
  d = item;
  owns = own;
}

QResource::QResource( const QString& filename )
{
  d = 0;
  owns = FALSE;

  QFile file( filename );
  if ( !file.open( IO_ReadOnly ) )
    return;

  uint size = file.size();
  char* buffer = new char[ size + 1 ];
  file.readBlock( buffer, size );
  file.close();
  buffer[ size ] = 0;
  //###### Torben: That will kill unicode. Better use QByteArray
  QString text( buffer, size + 1 );
  delete[] buffer;

  setContent( text );
}

void QResource::setContent( const QString& content )
{
  if ( d && owns )
    delete d;
  d = 0;

  QXMLSimpleParser parser;
  QResourceXMLConsumer consumer;

  if ( parser.parse( content, &consumer ) != -1 )
    return;

  d = consumer.donateTree();
  owns = TRUE;
}

void QResource::setContent( const QByteArray& array )
{
  //###### Torben: That will kill unicode!
  setContent( QString( array.data() ) );
}

QWidget* QResource::createWidget( QWidget* _parent ) const
{
  if ( !d )
    return 0;

  QMetaObject* m = QMetaObjectInit::metaObject( type() );
  if ( !m )
  {
    qDebug("Dont know class %s", type().ascii() );
    return 0;
  }

  if ( !m->inherits( "QWidget" ) )
  {
    qDebug("Class %s does not inherit from QWidget", type().ascii() );
    return 0;
  }

  QObjectFactory f = m->factory();
  if ( !f )
  {
    qDebug("Class %s has no factory", type().ascii() );
    return 0;
  }

  QWidget* w = (QWidget*)(*f)( _parent, *this );
  if ( !w )
  {
    qDebug("Factory for class %s returned 0", type().ascii() );
    return 0;
  }

  w->setConfiguration( *this );

  return w;
}

QLayout* QResource::createLayout( QWidget* _parent ) const
{
  if ( !d )
    return 0;

  QMetaObject* m = QMetaObjectInit::metaObject( type() );
  if ( !m )
  {
    qDebug("Dont know class %s", type().ascii() );
    return 0;
  }

  if ( !m->inherits( "QLayout" ) )
  {
    qDebug("Class %s does not inherit from QWidget", type().ascii() );
    return 0;
  }

  QObjectFactory f = m->factory();
  if ( !f )
  {
    qDebug("Class %s has no factory", type().ascii() );
    return 0;
  }

  QLayout* w = (QLayout*)(*f)( _parent, *this );
  if ( !w )
  {
    qDebug("Factory for class %s returned 0", type().ascii() );
    return 0;
  }

  w->setConfiguration( *this );

  return w;
}

QLayout* QResource::createLayout( QLayout* _parent ) const
{
  if ( !d )
    return 0;

  QMetaObject* m = QMetaObjectInit::metaObject( type() );
  if ( !m )
  {
    qDebug("Dont know class %s", type().ascii() );
    return 0;
  }

  if ( !m->inherits( "QLayout" ) )
  {
    qDebug("Class %s does not inherit from QWidget", type().ascii() );
    return 0;
  }

  QObjectFactory f = m->factory();
  if ( !f )
  {
    qDebug("Class %s has no factory", type().ascii() );
    return 0;
  }

  QLayout* w = (QLayout*)(*f)( _parent, *this );
  if ( !w )
  {
    qDebug("Factory for class %s returned 0", type().ascii() );
    return 0;
  }

  w->setConfiguration( *this );

  return w;
}

QLayout* QResource::createLayout() const
{
  if ( !d )
    return 0;

  QMetaObject* m = QMetaObjectInit::metaObject( type() );
  if ( !m )
  {
    qDebug("Dont know class %s", type().ascii() );
    return 0;
  }

  if ( !m->inherits( "QLayout" ) )
  {
    qDebug("Class %s does not inherit from QWidget", type().ascii() );
    return 0;
  }

  QObjectFactory f = m->factory();
  if ( !f )
  {
    qDebug("Class %s has no factory", type().ascii() );
    return 0;
  }

  QLayout* w = (QLayout*)(*f)( 0, *this );
  if ( !w )
  {
    qDebug("Factory for class %s returned 0", type().ascii() );
    return 0;
  }

  return w;
}

QStringList QResource::objectList()
{
  QStringList lst;
  if ( !d )
    return lst;

  QResourceItem* item = d->firstChild();
  for( ; item; item = item->nextSibling() )
    lst.append( item->type() );

  return lst;
}

QWidget* QResource::createWidget( QWidget* _parent, const QResourceItem* item )
{
  if ( !item )
    return 0;

  QResource resource( (QResourceItem*)item );
  return resource.createWidget( _parent );
}

QLayout* QResource::createLayout( QWidget* _parent, const QResourceItem* item )
{
  if ( !item )
    return 0;

  QResource resource( (QResourceItem*)item );
  return resource.createLayout( _parent );
}

QLayout* QResource::createLayout( QLayout* _parent, const QResourceItem* item )
{
  if ( !item )
    return 0;

  QResource resource( (QResourceItem*)item );
  return resource.createLayout( _parent );
}

QLayout* QResource::createLayout( const QResourceItem* item )
{
  if ( !item )
    return 0;

  QResource resource( (QResourceItem*)item );
  return resource.createLayout();
}


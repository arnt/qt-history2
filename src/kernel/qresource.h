/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qresource.h#5 $
**
** Definition of QResource classes
*
** Created : 980128
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
#ifndef QRESOURCE_H
#define QRESOURCE_H

#ifndef QT_H
#include "qstring.h"
#include "qmap.h"
#include "qproperty.h"
#include "qcolor.h"
#include "qstringlist.h"
#endif // QT_H

class QTextStream;
class QWidget;
class QLayout;
class QResourceItem;
class QResource;
class QResourceConstIterator;
struct QMetaProperty;


class Q_EXPORT QResourceIterator
{
  friend QResourceConstIterator;

public:
  QResourceIterator() { tag = 0; }
  QResourceIterator( const QResourceIterator& _it ) { tag = _it.tag; }
  QResourceIterator( QResourceItem* _tag ) { tag = _tag; }
  ~QResourceIterator() { };

  QResourceIterator operator++ (int);
  QResourceIterator& operator++ ();

  const QResourceItem& operator*() const { return *tag; }
  QResourceItem& operator*() { return *tag; }
  const QResourceItem* operator->() const { return tag; }
  QResourceItem* operator->() { return tag; }

  bool operator== ( const QResourceIterator &i ) const { return ( tag == i.tag ); }
  bool operator== ( const QResourceItem* t ) const { return ( tag == t ); }
  bool operator!= ( const QResourceIterator &i ) const { return ( tag != i.tag ); }
  bool operator!= ( const QResourceItem* t ) const { return ( tag != t ); }
  QResourceIterator& operator= ( const QResourceIterator &i ) { tag = i.tag; return *this; }
  QResourceIterator& operator= ( QResourceItem *t ) { tag = t; return *this; }

  bool operator! () const { return ( tag == 0 ); }
  operator bool() const { return ( tag != 0 ); }
  operator QResourceItem*() { return tag; }

  bool isValid() const { return ( tag != 0 ); }

protected:
  QResourceItem* tag;
};

class Q_EXPORT QResourceConstIterator
{
public:
  QResourceConstIterator() { tag = 0; }
  QResourceConstIterator( const QResourceIterator& _it ) { tag = _it.tag; }
  QResourceConstIterator( const QResourceConstIterator& _it ) { tag = _it.tag; }
  QResourceConstIterator( const QResourceItem* _tag ) { tag = _tag; }
  ~QResourceConstIterator() { };

  QResourceConstIterator operator++ (int);
  QResourceConstIterator& operator++ ();

  const QResourceItem& operator*() const { return *tag; }
  const QResourceItem* operator->() const { return tag; }

  bool operator== ( const QResourceConstIterator &i ) const { return ( tag == i.tag ); }
  bool operator== ( const QResourceIterator &i ) const { return ( tag == i.tag ); }
  bool operator== ( const QResourceItem* t ) const { return ( tag == t ); }
  bool operator!= ( const QResourceConstIterator &i ) const { return ( tag != i.tag ); }
  bool operator!= ( const QResourceIterator &i ) const { return ( tag != i.tag ); }
  bool operator!= ( const QResourceItem* t ) const { return ( tag != t ); }
  QResourceConstIterator& operator= ( const QResourceConstIterator &i ) { tag = i.tag; return *this; }
  QResourceConstIterator& operator= ( const QResourceIterator &i ) { tag = i.tag; return *this; }
  QResourceConstIterator& operator= ( const QResourceItem *t ) { tag = t; return *this; }

  bool operator! () const { return ( tag == 0 ); }
  operator bool() const { return ( tag != 0 ); }
  operator const QResourceItem*() const { return tag; }

  bool isValid() const { return ( tag != 0 ); }

protected:
  const QResourceItem* tag;
};

class Q_EXPORT QResourceItem
{
public:
  QResourceItem() { bIsText = FALSE; firstChildItem = nextSiblingItem = parentItem = 0; }
  QResourceItem( const QString& _type, bool _istext = FALSE );
  ~QResourceItem() { if ( firstChildItem ) delete firstChildItem; if ( nextSiblingItem ) delete nextSiblingItem; }

  bool isEmpty() const { return ( attribs.isEmpty() && !bIsText && firstChildItem == 0 ); }
  uint childCount() const;

  void clear();

  QResourceConstIterator begin() const { return QResourceConstIterator( firstChildItem ); }
  QResourceIterator begin() { return QResourceIterator( firstChildItem ); }
  QResourceConstIterator end() const { return QResourceConstIterator( 0 ); }
  QResourceIterator end() { return QResourceIterator( 0 ); }

  QResourceItem* nextSibling() { return nextSiblingItem; }
  const QResourceItem* nextSibling() const { return nextSiblingItem; }
  QResourceItem* parent() { return parentItem; }
  const QResourceItem* parent() const { return parentItem; }
  QResourceItem* firstChild() { return firstChildItem; }
  const QResourceItem* firstChild() const { return firstChildItem; }

  bool isText() const { return bIsText; }
  QString text() const { if ( bIsText ) return txt; return QString::null; }
  QString type() const { if ( !bIsText ) return txt; return QString::null; }
  QString name() const { if ( attribs.contains( "name" ) ) return attribs[ "name" ]; return QString::null; }

  void setText( const QString& _t ) { txt = _t; bIsText = TRUE; attribs.clear(); if ( firstChildItem ) delete firstChildItem; firstChildItem = 0; }
  void setType( const QString& _t ) { txt = _t; bIsText = FALSE; }
  void setName( const QString& _t ) { if ( bIsText ) attribs.insert( "name", _t ); }

  void removeAttrib( const QString& _name ) { attribs.remove( _name ); }
  void insertAttrib( const QString& _name, const QString& _value ) { attribs.insert( _name, _value ); }
  void insertAttrib( const QString& _name, const char* value ) { attribs.insert( _name, value ); }
  void insertAttrib( const QString& _name, int i ) { QString tmp; attribs.insert( _name, tmp.setNum( i ) ); }
  void insertAttrib( const QString& _name, bool b ) { QString tmp; attribs.insert( _name, ( b ? "true" : "false" ) ); }

  void insert( const QResourceItem* pos, QResourceItem* item );
  void prepend( QResourceItem* _item );
  void append( QResourceItem* _item ) { insert( 0, _item ); }
  void remove( const QResourceItem* old ) { QResourceItem* t = extract( old ); if ( t ) delete t; }
  void replace( const QResourceItem* pos, QResourceItem* t );
  QResourceItem* extractAndReplace( const QResourceItem* pos, QResourceItem* t );
  QResourceItem* extract( const QResourceItem* pos );

  QResourceItem* child( const QString& type );
  const QResourceItem* child( const QString& type ) const;

  QProperty property( const QMetaProperty* ) const;
  QProperty property( const QString& name, QProperty::Type type ) const;
  void setProperty( const QString& name, const QProperty& prop );

  const QMap<QString,QString>& attribsMap() const { return attribs; }
  QMap<QString,QString>& attribsMap() { return attribs; }

  bool hasAttrib( const QString& _name ) const;
  QString attrib( const QString& _name ) const { return attribs[ _name ]; }
  int intAttrib( const QString& _name ) const;
  bool boolAttrib( const QString& _name ) const;
  QColor colorAttrib( const QString& _name ) const;
  double doubleAttrib( const QString& _name ) const;

private:
  bool stringProperty( const QString& _name, QString* _val ) const;

  QResourceItem* parentItem;
  QResourceItem* firstChildItem;
  QResourceItem* nextSiblingItem;
  QString txt;
  QMap<QString,QString> attribs;
  bool bIsText;
};

class Q_EXPORT QResource
{
public:
  QResource();
  QResource( const QString& filename );
  QResource( QResourceItem* item, bool own = FALSE );
  ~QResource() { if ( d && owns ) delete d; }

  void setContent( const QString& content );
  void setContent( const QByteArray& array );

  typedef QResourceIterator Iterator;
  typedef QResourceConstIterator ConstIterator;
  typedef QResourceItem ValueType;

  QString type() const { if ( !d ) return QString::null; return d->type(); }

  QWidget* createWidget( QWidget* _parent ) const;
  QLayout* createLayout( QWidget* _parent ) const;
  QLayout* createLayout( QLayout* _parent ) const;
  QLayout* createLayout() const;

  QStringList objectList();
  bool contains( const QString& name ) { return objectList().contains( name ); }

  QResourceItem* tree() { return d; }
  const QResourceItem* tree() const { return d; }

  bool isEmpty() const { return ( d == 0 ); }

  static QWidget* createWidget( QWidget* _parent, const QResourceItem* );
  static QLayout* createLayout( QWidget* _parent, const QResourceItem* );
  static QLayout* createLayout( QLayout* _parent, const QResourceItem* );
  static QLayout* createLayout( const QResourceItem* );

private:
  QResourceItem* d;
  bool owns;

#if defined(Q_DISABLE_COPY)
  QResource( const QResource & );
  QResource &operator=( const QResource & );
#endif
};

Q_EXPORT QTextStream& operator<< ( QTextStream&, const QResourceItem& tag );
Q_EXPORT QTextStream& operator<< ( QTextStream&, const QResource& );
Q_EXPORT QTextStream& operator>> ( QTextStream&, QResource& );

#endif

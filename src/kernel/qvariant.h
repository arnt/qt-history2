/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qvariant.h#4 $
**
** Definition of QVariant class
**
** Created : 990414
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
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

#ifndef QVARIANT_H
#define QVARIANT_H

#if defined(Q_TEMPLATE_NEEDS_CLASS_DECLARATION) && ( defined(QVALUELIST_H) || defined(QMAP_H) )
#warning Qt warning:
#warning Header file qvaluelist.h or qmap.h is included before qvariant.h.
#warning This will cause errors on this compiler because of improper template support.
#warning Try changing the order of inclusion of your header files.
#endif

#ifndef QT_H
#include "qstring.h"
#endif // QT_H

class QString;
class QCString;
class QFont;
class QPixmap;
class QBrush;
class QRect;
class QPoint;
class QImage;
class QSize;
class QColor;
class QPalette;
class QColorGroup;
class QIconSet;
class QDataStream;
class QPointArray;
class QRegion;
class QBitmap;
class QCursor;
class QStringList;
// Relevant header files rejected after QVariant declaration
// for GCC 2.7.* compatibility
class QVariant;
class QVariantPrivate;
class QVariantTypeBase;
class QVariantValueBase;
template <class T> class QValueList;
template <class T> class QValueListConstIterator;
template <class T> class QValueListNode;
template <class Key, class T> class QMap;
template <class Key, class T> class QMapConstIterator;

/**
 * This class acts like a union. It can hold one value at the
 * time but it can hold the most common types.
 * For CORBA people: It is a poor mans CORBA::Any.
 */
class Q_EXPORT QVariant
{
public:
    enum Type {
	Invalid,
	Map,
	List,
	String,
	StringList,
	Font,
	Pixmap,
	Brush,
	Rect,
	Size,
	Color,
	Palette,
	ColorGroup,
	IconSet,
	Point,
	Image,
	Int,
	UInt,
	Bool,
	Double,
	CString,
	PointArray,
	Region,
	Bitmap,
	Cursor,
	Custom
    };

    QVariant();
    ~QVariant();
    QVariant( const QVariant& );
    QVariant( QDataStream& s );

    QVariant( const QString& );
    QVariant( const QCString& );
    QVariant( const char* );
    QVariant( const QStringList& );
    QVariant( const QFont& );
    QVariant( const QPixmap& );
    QVariant( const QImage& );
    QVariant( const QBrush& );
    QVariant( const QPoint& );
    QVariant( const QRect& );
    QVariant( const QSize& );
    QVariant( const QColor& );
    QVariant( const QPalette& );
    QVariant( const QColorGroup& );
    QVariant( const QIconSet& );
    QVariant( const QPointArray& );
    QVariant( const QRegion& );
    QVariant( const QBitmap& );
    QVariant( const QCursor& );
    QVariant( const QValueList<QVariant>& );
    QVariant( const QMap<QString,QVariant>& );
    QVariant( int );
    QVariant( uint );
    // ### Problems on some compilers ?
    QVariant( bool );
    QVariant( double );
    QVariant( void* custom, const QVariantTypeBase* type );
    QVariant( const QVariantValueBase& );

    QVariant& operator= ( const QVariant& );

    Type type() const;
    const char* typeName() const;
    const QVariantTypeBase* customType() const;

    bool canCast( Type ) const;
    bool canCast( const QVariantTypeBase* type ) const;

    bool isValid() const;

    void clear();

    const QString toString() const;
    const QCString toCString() const;
    const QStringList toStringList() const;
    const QFont toFont() const;
    const QPixmap toPixmap() const;
    const QImage toImage() const;
    const QBrush toBrush() const;
    const QPoint toPoint() const;
    const QRect toRect() const;
    const QSize toSize() const;
    const QColor toColor() const;
    const QPalette toPalette() const;
    const QColorGroup toColorGroup() const;
    const QIconSet toIconSet() const;
    const QPointArray toPointArray() const;
    const QBitmap toBitmap() const;
    const QRegion toRegion() const;
    const QCursor toCursor() const;
    int toInt() const;
    uint toUInt() const;
    bool toBool() const;
    double toDouble() const;
    const QValueList<QVariant> toList() const;
    const QMap<QString,QVariant> toMap() const;
    void* toCustom( const QVariantTypeBase* type ) const;

    QValueListConstIterator<QVariant> listBegin() const;
    QValueListConstIterator<QVariant> listEnd() const;
    QValueListConstIterator<QString> stringListBegin() const;
    QValueListConstIterator<QString> stringListEnd() const;
    QMapConstIterator<QString,QVariant> mapBegin() const;
    QMapConstIterator<QString,QVariant> mapEnd() const;
    QMapConstIterator<QString,QVariant> mapFind( const QString& ) const;

    QString& asString();
    QCString& asCString();
    QStringList& asStringList();
    QFont& asFont();
    QPixmap& asPixmap();
    QImage& asImage();
    QBrush& asBrush();
    QPoint& asPoint();
    QRect& asRect();
    QSize& asSize();
    QColor& asColor();
    QPalette& asPalette();
    QColorGroup& asColorGroup();
    QIconSet& asIconSet();
    QPointArray& asPointArray();
    QBitmap& asBitmap();
    QRegion& asRegion();
    QCursor& asCursor();
    int& asInt();
    uint& asUInt();
    bool& asBool();
    double& asDouble();
    QValueList<QVariant>& asList();
    QMap<QString,QVariant>& asMap();
    void* asCustom( const QVariantTypeBase* );
    void* asCustom();
    const void* asCustom() const;

    void load( QDataStream& );
    void save( QDataStream& ) const;

    static const char* typeToName( Type typ );
    static Type nameToType( const char* name );

private:
    void detach();

    QVariantPrivate* d;
};

#if defined(Q_FULL_TEMPLATE_INSTANTIATION)
    bool operator== ( const QVariant&, const QVariant& );
#endif

class QVariantPrivate : public QShared
{
public:
    QVariantPrivate();
    QVariantPrivate( QVariantPrivate* );
    ~QVariantPrivate();

    void clear();

    QVariant::Type typ;
    union
    {
	uint u;
	int i;
	bool b;
	double d;
	void *ptr;
    } value;

    const QVariantTypeBase* custom_type;
};

class QVariantTypeBase
{
public:
    QVariantTypeBase( const char* type );
    virtual ~QVariantTypeBase();

    virtual void* create() const = 0;
    virtual void destroy( void* ) const = 0;
    virtual void* copy( const void* ) const = 0;

    virtual void save( const void*, QDataStream& ) const = 0;
    virtual void load( void*, QDataStream& ) const = 0;

    virtual void* castFrom( const QVariant& ) const;
    virtual QVariant castTo( const void* ptr, QVariant::Type ) const;
    virtual void* castTo( const void* ptr, const QVariantTypeBase* type ) const;

    virtual bool canCastTo( const QVariantTypeBase* type ) const;
    virtual bool canCastTo( QVariant::Type ) const;
    virtual bool canCastFrom( const QVariantTypeBase* type ) const;
    virtual bool canCastFrom( QVariant::Type ) const;

    const char* typeName() const;

    static const QVariantTypeBase* type( const char* t );

private:
    QCString m_type;
};

/**
 * Template for convenience.
 */
template <class T>
class QVariantType : public QVariantTypeBase
{
public:
    QVariantType( const char* type ) : QVariantTypeBase( type ) { };
    ~QVariantType() { };

    void* create() const { return new T; }
    void destroy( void* ptr ) const { delete (T*)ptr; }
    void* copy( const void* ptr ) const 
        { return new T( (const T&)*((const T*)ptr) ); }
    void save( const void* ptr, QDataStream& str ) const { str << *((T*)ptr); }
    void load( void* ptr, QDataStream& str ) const { str >> *((T*)ptr); }
};

/**
 * Dieses Ding wird in QVariant gespeichert.
 * CopyConstructor und Delete funktionieren mit Hilfe von QVariantTypeBase
 */
class QVariantValueBase
{
public:
    QVariantValueBase( const QVariantTypeBase* t ) { ptr = 0; typ = t; }
    QVariantValueBase( void* p, const QVariantTypeBase* t ) { ptr = p; typ = t; }
    QVariantValueBase( const QVariantValueBase& p ) { typ = p.typ; if ( typ ) ptr = typ->copy( p.ptr ); else ptr = 0; }
    ~QVariantValueBase() { if ( ptr && typ ) typ->destroy( ptr ); }

    bool isNull() const { return ( ptr == 0 || typ == 0 ); }

    void* value() { return ptr; }
    const void* value() const { return ptr; }
    const QVariantTypeBase* type() const { return typ; }

protected:
    void assign( const QVariant& v ) { if ( ptr ) typ->destroy( ptr ); ptr = v.toCustom( typ ); }
    void assign( void* p ) { if ( ptr ) typ->destroy( ptr ); ptr = p; }
    void assign( QVariantValueBase& p ) { if ( ptr ) typ->destroy( ptr ); typ = p.typ; if ( typ ) ptr = typ->copy( p.ptr ); else ptr = 0; }

private:
    void* ptr;
    const QVariantTypeBase* typ;
};

template< class V, class T >
class QVariantValue : public QVariantValueBase
{
public:
    QVariantValue() : QVariantValueBase( T::type() ) { }
    QVariantValue( const QVariant& v ) : QVariantValueBase( T::type() ) { assign( v ); }
    QVariantValue( const V& v ) : QVariantValueBase( new V( v ), T::type() ) { }
    QVariantValue( const QVariantValue<V,T>& ptr ) : QVariantValueBase( ptr ) { }

    QVariantValue<V,T>& operator=( const QVariantValue<V,T>& v ) { assign( v ); return *this; }
    QVariantValue<V,T>& operator=( const QVariant& v ) { assign( v ); return *this; }
    QVariantValue<V,T>& operator=( const V& v ) { assign( new V( v ) ); return *this; }

    const V* operator->() const { return (V*)value(); }
    V* operator->() { return (V*)value(); }
    const V& operator*() const { return *((V*)value()); }
    V& operator*() { return *((V*)value()); }
    operator const V*() const { return (V*)value(); }
    operator V*() { return (V*)value(); }

private:
    bool operator==( QVariantValue<V,T>& v ) { return FALSE; }
    bool operator!=( QVariantValue<V,T>& v ) { return FALSE; }
};

// These header files are down here for GCC 2.7.* compatibility
#ifndef QT_H
#include "qvaluelist.h"
#include "qstringlist.h"
#include "qmap.h"
#endif // QT_H

inline QVariant::Type QVariant::type() const
{
    return d->typ;
}

inline bool QVariant::isValid() const
{
    return (d->typ != Invalid);
}

inline QValueListConstIterator<QString> QVariant::stringListBegin() const
{
    if ( d->typ != StringList )
	return QValueListConstIterator<QString>();
    return ((const QStringList*)d->value.ptr)->begin();
}

inline QValueListConstIterator<QString> QVariant::stringListEnd() const
{
    if ( d->typ != StringList )
	return QValueListConstIterator<QString>();
    return ((const QStringList*)d->value.ptr)->end();
}

inline QValueListConstIterator<QVariant> QVariant::listBegin() const
{
    if ( d->typ != List )
	return QValueListConstIterator<QVariant>();
    return ((const QValueList<QVariant>*)d->value.ptr)->begin();
}

inline QValueListConstIterator<QVariant> QVariant::listEnd() const
{
    if ( d->typ != List )
	return QValueListConstIterator<QVariant>();
    return ((const QValueList<QVariant>*)d->value.ptr)->end();
}

inline QMapConstIterator<QString,QVariant> QVariant::mapBegin() const
{
    if ( d->typ != Map )
	return QMapConstIterator<QString,QVariant>();
    return ((const QMap<QString,QVariant>*)d->value.ptr)->begin();
}

inline QMapConstIterator<QString,QVariant> QVariant::mapEnd() const
{
    if ( d->typ != Map )
	return QMapConstIterator<QString,QVariant>();
    return ((const QMap<QString,QVariant>*)d->value.ptr)->end();
}

inline QMapConstIterator<QString,QVariant> QVariant::mapFind( const QString& key ) const
{
    if ( d->typ != Map )
	return QMapConstIterator<QString,QVariant>();
    return ((const QMap<QString,QVariant>*)d->value.ptr)->find( key );
}

Q_EXPORT QDataStream& operator>> ( QDataStream& s, QVariant& p );
Q_EXPORT QDataStream& operator<< ( QDataStream& s, const QVariant& p );
Q_EXPORT QDataStream& operator>> ( QDataStream& s, QVariant::Type& p );
Q_EXPORT QDataStream& operator<< ( QDataStream& s, const QVariant::Type p );


#endif

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
#warning Headers qvaluelist.h or qmap.h are included before qvariant.h.
#warning The compiler you are using lacks proper template support.
#warning You will probably see errors now...
#warning Try changing the order of inclusion of your header files.
#endif

#ifndef QT_H
#include "qstring.h"
#include "qshared.h"
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
// Relevant header files removed from above for GCC 2.7.* compatibility, so...
class QVariant;
class QVariantTypeBase;
class QVariantValueBase;
class QStringList;
template <class T> class QValueList;
template <class T> struct QValueListNode;
template <class Key, class T> class QMap;


/**
 * This class acts like a union. It can hold one value at the
 * time but it can hold the most common types.
 * For CORBA people: It is a poor mans CORBA::Any.
 */
class Q_EXPORT QVariant : public QShared
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
    QVariant& operator= ( const QString& );
    QVariant& operator= ( const QCString& );
    QVariant& operator= ( const char* );
    QVariant& operator= ( const QStringList& );
    QVariant& operator= ( const QFont& );
    QVariant& operator= ( const QPixmap& );
    QVariant& operator= ( const QImage& );
    QVariant& operator= ( const QBrush& );
    QVariant& operator= ( const QPoint& );
    QVariant& operator= ( const QRect& );
    QVariant& operator= ( const QSize& );
    QVariant& operator= ( const QColor& );
    QVariant& operator= ( const QPalette& );
    QVariant& operator= ( const QColorGroup& );
    QVariant& operator= ( const QIconSet& );
    QVariant& operator= ( const QValueList<QVariant>& );
    QVariant& operator= ( const QMap<QString,QVariant>& );
    QVariant& operator= ( int );
    QVariant& operator= ( uint );
    QVariant& operator= ( double );
    QVariant& operator= ( const QVariantValueBase& );

    void setValue( const QString& );
    void setValue( const QCString& );
    void setValue( const char* );
    void setValue( const QStringList& );
    void setValue( const QFont& );
    void setValue( const QPixmap& );
    void setValue( const QImage& );
    void setValue( const QBrush& );
    void setValue( const QPoint& );
    void setValue( const QRect& );
    void setValue( const QSize& );
    void setValue( const QColor& );
    void setValue( const QPalette& );
    void setValue( const QColorGroup& );
    void setValue( const QIconSet& );
    void setValue( const QValueList<QVariant>& );
    void setValue( const QMap<QString,QVariant>& );
    void setValue( int );
    void setValue( uint );
    void setBoolValue( bool );
    void setValue( double );
    void setValue( void* custom, const QVariantTypeBase* type );
    void setValue( const QVariantValueBase& );

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
    int toInt() const;
    uint toUInt() const;
    bool toBool() const;
    double toDouble() const;
    const QValueList<QVariant> toList() const;
    const QMap<QString,QVariant> toMap() const;
    void* toCustom( const QVariantTypeBase* type ) const;

    const QStringList& asStringList() const;
    const QValueList<QVariant>& asList() const;
    const QMap<QString,QVariant>& asMap() const;
    const void* asCustom() const;

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
    int& asInt();
    uint& asUInt();
    bool& asBool();
    double& asDouble();
    QValueList<QVariant>& asList();
    QMap<QString,QVariant>& asMap();
    void* asCustom( const QVariantTypeBase* );
    void* asCustom();

    void load( QDataStream& );
    void save( QDataStream& ) const;

    static const char* typeToName( Type typ );
    static Type nameToType( const char* name );

private:
    Type typ;
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
    void* copy( const void* ptr ) const { return new T( (const T&)*ptr ); }

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
    return typ;
}

inline bool QVariant::isValid() const
{
    return (typ != Invalid);
}

Q_EXPORT QDataStream& operator>> ( QDataStream& s, QVariant& p );
Q_EXPORT QDataStream& operator<< ( QDataStream& s, const QVariant& p );
Q_EXPORT QDataStream& operator>> ( QDataStream& s, QVariant::Type& p );
Q_EXPORT QDataStream& operator<< ( QDataStream& s, const QVariant::Type p );

#if defined(Q_FULL_TEMPLATE_INSTANTIATION)
Q_EXPORT bool operator== ( const QVariant&, const QVariant& );
#endif

#endif

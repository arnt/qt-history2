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

#if defined(Q_TEMPLATE_NEEDS_CLASS_DECLARATION) && defined(QVALUELIST_H)
#warning "This compiler will not let you include qvaluelist.h before qvariant.h"
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
// Relevant header files removed from above for GCC 2.7.* compatibility, so...
class QVariant;
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
	NTypes=CString,
	Custom = 0x1000
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
    QVariant( bool );
    QVariant( double );

    QVariant& operator= ( const QVariant& );
    
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

    Type type() const;
    const char* typeName() const;

    bool canCast( Type ) const;

    bool isValid() const;

    QString toString() const;
    QCString toCString() const;
    QStringList toStringList() const;
    QFont toFont() const;
    QPixmap toPixmap() const;
    QImage toImage() const;
    QBrush toBrush() const;
    QPoint toPoint() const;
    QRect toRect() const;
    QSize toSize() const;
    QColor toColor() const;
    QPalette toPalette() const;
    QColorGroup toColorGroup() const;
    QIconSet toIconSet() const;
    int toInt() const;
    uint toUInt() const;
    bool toBool() const;
    double toDouble() const;
    QValueList<QVariant> toList() const;
    QMap<QString,QVariant> toMap() const;

    void load( QDataStream& );
    void save( QDataStream& ) const;

    static const char* typeToName( Type typ );
    static Type nameToType( const char* name );

protected:
    void clear();

    Type typ;
    union
    {
	uint u;
	int i;
	bool b;
	double d;
	void *ptr;
    } value;

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

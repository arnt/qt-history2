/****************************************************************************
**
** Implementation of QVariant class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qvariant.h"
#ifndef QT_NO_VARIANT
#include "qbitarray.h"
#include "qbitmap.h"
#include "qbrush.h"
#include "qbytearray.h"
#include "qcolor.h"
#include "qcursor.h"
#include "qdatastream.h"
#include "qdatetime.h"
#include "qfont.h"
#include "qiconset.h"
#include "qimage.h"
#include "qkeysequence.h"
#include "qlist.h"
#include "qmap.h"
#include "qpalette.h"
#include "qpen.h"
#include "qpixmap.h"
#include "qpoint.h"
#include "qpointarray.h"
#include "qrect.h"
#include "qregion.h"
#include "qsize.h"
#include "qsizepolicy.h"
#include "qstring.h"

#include <float.h>

#ifndef DBL_DIG
#define DBL_DIG 10
#endif //DBL_DIG


void QVariant::Private::clear()
{
    switch (type) {
    case QVariant::Bitmap:
	delete static_cast<QBitmap *>(value.ptr);
	break;
    case QVariant::Cursor:
	delete static_cast<QCursor *>(value.ptr);
	break;
    case QVariant::Region:
	delete static_cast<QRegion *>(value.ptr);
	break;
    case QVariant::PointArray:
	delete static_cast<QPointArray *>(value.ptr);
	break;
    case QVariant::String:
	delete static_cast<QString *>(value.ptr);
	break;
#ifndef QT_NO_STRINGLIST
    case QVariant::StringList:
	delete static_cast<QStringList *>(value.ptr);
	break;
#endif //QT_NO_STRINGLIST
    case QVariant::Font:
	delete static_cast<QFont *>(value.ptr);
	break;
    case QVariant::Pixmap:
	delete static_cast<QPixmap *>(value.ptr);
	break;
    case QVariant::Image:
	delete static_cast<QImage *>(value.ptr);
	break;
    case QVariant::Brush:
	delete static_cast<QBrush *>(value.ptr);
	break;
    case QVariant::Point:
	delete static_cast<QPoint *>(value.ptr);
	break;
    case QVariant::Rect:
	delete static_cast<QRect *>(value.ptr);
	break;
    case QVariant::Size:
	delete static_cast<QSize *>(value.ptr);
	break;
    case QVariant::Color:
	delete static_cast<QColor *>(value.ptr);
	break;
#ifndef QT_NO_PALETTE
    case QVariant::Palette:
	delete static_cast<QPalette *>(value.ptr);
	break;
#ifndef QT_NO_COMPAT
    case QVariant::ColorGroup:
	delete static_cast<QColorGroup *>(value.ptr);
	break;
#endif
#endif
#ifndef QT_NO_ICONSET
    case QVariant::IconSet:
	delete static_cast<QIconSet *>(value.ptr);
	break;
#endif
#ifndef QT_NO_TEMPLATE_VARIANT
    case QVariant::Map:
	delete static_cast<QMap<QString, QVariant> *>(value.ptr);
	break;
    case QVariant::List:
	delete static_cast<QList<QVariant> *>(value.ptr);
	break;
#endif
    case QVariant::SizePolicy:
	delete static_cast<QSizePolicy *>(value.ptr);
	break;
    case QVariant::Date:
	delete static_cast<QDate *>(value.ptr);
	break;
    case QVariant::Time:
	delete static_cast<QTime *>(value.ptr);
	break;
    case QVariant::DateTime:
	delete static_cast<QDateTime *>(value.ptr);
	break;
    case QVariant::ByteArray:
	delete static_cast<QByteArray *>(value.ptr);
	break;
    case QVariant::BitArray:
	delete static_cast<QBitArray *>(value.ptr);
	break;
#ifndef QT_NO_ACCEL
    case QVariant::KeySequence:
	delete static_cast<QKeySequence *>(value.ptr);
	break;
#endif
    case QVariant::Pen:
	delete static_cast<QPen *>(value.ptr);
	break;
    case QVariant::Invalid:
    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::LongLong:
    case QVariant::ULongLong:
    case QVariant::Bool:
    case QVariant::Double:
	break;
    }

    type = QVariant::Invalid;
    is_null = true;
}

/*!
    \class QVariant qvariant.h
    \brief The QVariant class acts like a union for the most common Qt data types.

    \ingroup objectmodel
    \ingroup misc
    \mainclass

    Because C++ forbids unions from including types that have
    non-default constructors or destructors, most interesting Qt
    classes cannot be used in unions. Without QVariant, this would be
    a problem for QObject::property() and for database work, etc.

    A QVariant object holds a single value of a single type() at a
    time. (Some type()s are multi-valued, for example a string list.)
    You can find out what type, T, the variant holds, convert it to a
    different type using one of the asT() functions, e.g. asSize(),
    get its value using one of the toT() functions, e.g. toSize(), and
    check whether the type can be converted to a particular type using
    canCast().

    The methods named toT() (for any supported T, see the \c Type
    documentation for a list) are const. If you ask for the stored
    type, they return a copy of the stored object. If you ask for a
    type that can be generated from the stored type, toT() copies and
    converts and leaves the object itself unchanged. If you ask for a
    type that cannot be generated from the stored type, the result
    depends on the type (see the function documentation for details).

    Note that two data types supported by QVariant are explicitly
    shared, namely QImage and QPointArray, and in these
    cases the toT() methods return a shallow copy. In almost all cases
    you must make a deep copy of the returned values before modifying
    them.

    The asT() functions are not const. They do conversion like the
    toT() methods, set the variant to hold the converted value, and
    return a reference to the new contents of the variant.

    Here is some example code to demonstrate the use of QVariant:

    \code
    QDataStream out(...);
    QVariant v(123);          // The variant now contains an int
    int x = v.toInt();        // x = 123
    out << v;                 // Writes a type tag and an int to out
    v = QVariant("hello");    // The variant now contains a QByteArray
    v = QVariant(tr("hello"));// The variant now contains a QString
    int y = v.toInt();        // y = 0 since v cannot be converted to an int
    QString s = v.toString(); // s = tr("hello")  (see QObject::tr())
    out << v;                 // Writes a type tag and a QString to out
    ...
    QDataStream in(...);      // (opening the previously written stream)
    in >> v;                  // Reads an Int variant
    int z = v.toInt();        // z = 123
    qDebug("Type is %s",      // prints "Type is int"
	    v.typeName());
    v.asInt() += 100;	      // The variant now hold the value 223.
    v = QVariant( QStringList() );
    v.asStringList().append( "Hello" );
    \endcode

    You can even store QList<QVariant>s and
    QMap<QString,QVariant>s in a variant, so you can easily construct
    arbitrarily complex data structures of arbitrary types. This is
    very powerful and versatile, but may prove less memory and speed
    efficient than storing specific types in standard data structures.

    QVariant also supports the notion of NULL values, where you have a
    defined type with no value set.
    \code
    QVariant x, y( QString() ), z( QString("") );
    x.asInt();
    // x.isNull() == true, y.isNull() == true, z.isNull() == false
    \endcode

    See the \link collection.html Collection Classes\endlink.
*/

/*!
    \enum QVariant::Type

    This enum type defines the types of variable that a QVariant can
    contain.

    \value Invalid  no type
    \value BitArray  a QBitArray
    \value ByteArray  a QByteArray
    \value Bitmap  a QBitmap
    \value Bool  a bool
    \value Brush  a QBrush
    \value Color  a QColor
    \value Cursor  a QCursor
    \value Date  a QDate
    \value DateTime  a QDateTime
    \value Double  a double
    \value Font  a QFont
    \value IconSet  a QIconSet
    \value Image  a QImage
    \value Int  an int
    \value KeySequence  a QKeySequence
    \value List  a QList<QVariant>
    \value LongLong a long long
    \value ULongLong an unsigned long long
    \value Map  a QMap<QString,QVariant>
    \value Palette  a QPalette
    \value Pen  a QPen
    \value Pixmap  a QPixmap
    \value Point  a QPoint
    \value PointArray  a QPointArray
    \value Rect  a QRect
    \value Region  a QRegion
    \value Size  a QSize
    \value SizePolicy  a QSizePolicy
    \value String  a QString
    \value StringList  a QStringList
    \value Time  a QTime
    \value UInt  an unsigned int

    Note that Qt's definition of bool depends on the compiler.
    \c qglobal.h has the system-dependent definition of bool.
*/

/*!
    Constructs an invalid variant.
*/

QVariant::Private QVariant::shared_invalid = { Q_ATOMIC_INIT(1), Invalid, true, {0} };

QVariant::QVariant()
    :d(&shared_invalid)
{
    ++d->ref;
}

/*!
    \internal

    Constructs a variant of type \a type, and initializes with \a v if
    \a not 0.
*/
QVariant::QVariant(Type type, void *v)
{
    d = (v ? constructPrivate(type, v) : constructPrivate(type));
}

/*!
    Destroys the QVariant and the contained object.

    Note that subclasses that reimplement clear() should reimplement
    the destructor to call clear(). This destructor calls clear(), but
    because it is the destructor, QVariant::clear() is called rather
    than a subclass's clear().
*/
QVariant::~QVariant()
{
    if (!--d->ref)
	cleanUp(d);
}

/*!
    Constructs a copy of the variant, \a p, passed as the argument to
    this constructor. Usually this is a deep copy, but a shallow copy
    is made if the stored data type is explicitly shared, as e.g.
    QImage is.
*/
QVariant::QVariant(const QVariant &p)
    : d(p.d)
{
    ++d->ref;
}

#ifndef QT_NO_DATASTREAM
/*!
    Reads the variant from the data stream, \a s.
*/
QVariant::QVariant(QDataStream &s)
{
    d = startConstruction();
    s >> *this;
}
#endif //QT_NO_DATASTREAM

/*!
    Constructs a new variant with a string value, \a val.
*/
QVariant::QVariant(const QString &val)
{
    d = startConstruction();
    d->type = String;
    d->value.ptr = new QString(val);
}

/*!
    Constructs a new variant with a C-string value of \a val if \a val
    is non-null. The variant creates a deep copy of \a val.

    If \a val is null, the resulting variant has type Invalid.
*/
QVariant::QVariant(const char *val)
{
    d = startConstruction();
    if ( val == 0 )
	return;
    d->type = ByteArray;
    d->value.ptr = new QByteArray(val);
}

#ifndef QT_NO_STRINGLIST
/*!
    Constructs a new variant with a string list value, \a val.
*/
QVariant::QVariant(const QStringList &val)
{
    d = startConstruction();
    d->type = StringList;
    d->value.ptr = new QStringList(val);
    d->is_null = false;
}
#endif // QT_NO_STRINGLIST

#ifndef QT_NO_TEMPLATE_VARIANT
/*!
    Constructs a new variant with a map of QVariants, \a val.
*/
QVariant::QVariant(const QMap<QString,QVariant> &val)
{
    d = startConstruction();
    d->type = Map;
    d->value.ptr = new QMap<QString,QVariant>(val);
    d->is_null = false;
}
#endif
/*!
    Constructs a new variant with a font value, \a val.
*/
QVariant::QVariant(const QFont &val)
{
    d = startConstruction();
    d->type = Font;
    d->value.ptr = new QFont(val);
    d->is_null = false;
}

/*!
    Constructs a new variant with a pixmap value, \a val.
*/
QVariant::QVariant(const QPixmap &val)
{
    d = startConstruction();
    d->type = Pixmap;
    d->value.ptr = new QPixmap(val);
}


/*!
    Constructs a new variant with an image value, \a val.

    Because QImage is explicitly shared, you may need to pass a deep
    copy to the variant using QImage::copy(), e.g. if you intend
    changing the image you've passed later on.
*/
QVariant::QVariant(const QImage &val)
{
    d = startConstruction();
    d->type = Image;
    d->value.ptr = new QImage(val);
}

/*!
    Constructs a new variant with a brush value, \a val.
*/
QVariant::QVariant(const QBrush &val)
{
    d = startConstruction();
    d->type = Brush;
    d->value.ptr = new QBrush(val);
    d->is_null = false;
}

/*!
    Constructs a new variant with a point value, \a val.
*/
QVariant::QVariant(const QPoint &val)
{
    d = startConstruction();
    d->type = Point;
    d->value.ptr = new QPoint(val);
}

/*!
    Constructs a new variant with a rect value, \a val.
*/
QVariant::QVariant(const QRect &val)
{
    d = startConstruction();
    d->type = Rect;
    d->value.ptr = new QRect(val);
}

/*!
    Constructs a new variant with a size value, \a val.
*/
QVariant::QVariant(const QSize &val)
{
    d = startConstruction();
    d->type = Size;
    d->value.ptr = new QSize(val);
}

/*!
    Constructs a new variant with a color value, \a val.
*/
QVariant::QVariant(const QColor &val)
{
    d = startConstruction();
    d->type = Color;
    d->value.ptr = new QColor(val);
    d->is_null = false;
}

#ifndef QT_NO_PALETTE
/*!
    Constructs a new variant with a color palette value, \a val.
*/
QVariant::QVariant(const QPalette &val)
{
    d = startConstruction();
    d->type = Palette;
    d->value.ptr = new QPalette(val);
    d->is_null = false;
}

#ifndef QT_NO_COMPAT
QVariant::QVariant(const QColorGroup &val)
{
    d = startConstruction();
    d->type = ColorGroup;
    d->value.ptr = new QColorGroup(val);
    d->is_null = false;
}
#endif

#endif //QT_NO_PALETTE
#ifndef QT_NO_ICONSET
/*!
    Constructs a new variant with an icon set value, \a val.
*/
QVariant::QVariant(const QIconSet &val)
{
    d = startConstruction();
    d->type = IconSet;
    d->value.ptr = new QIconSet(val);
}
#endif //QT_NO_ICONSET
/*!
    Constructs a new variant with a region value, \a val.
*/
QVariant::QVariant(const QRegion &val)
{
    d = startConstruction();
    d->type = Region;
    // ## Force a detach
    d->value.ptr = new QRegion(val);
    static_cast<QRegion *>(d->value.ptr)->translate(0, 0);
}

/*!
    Constructs a new variant with a bitmap value, \a val.
*/
QVariant::QVariant(const QBitmap& val)
{
    d = startConstruction();
    d->type = Bitmap;
    d->value.ptr = new QBitmap(val);
}

/*!
    Constructs a new variant with a cursor value, \a val.
*/
QVariant::QVariant(const QCursor &val)
{
    d = startConstruction();
    d->type = Cursor;
    d->value.ptr = new QCursor(val);
    d->is_null = false;
}

/*!
    Constructs a new variant with a point array value, \a val.

    Because QPointArray is explicitly shared, you may need to pass a
    deep copy to the variant using QPointArray::copy(), e.g. if you
    intend changing the point array you've passed later on.
*/
QVariant::QVariant(const QPointArray &val)
{
    d = startConstruction();
    d->type = PointArray;
    d->value.ptr = new QPointArray(val);
}

/*!
    Constructs a new variant with a date value, \a val.
*/
QVariant::QVariant(const QDate &val)
{
    d = startConstruction();
    d->type = Date;
    d->value.ptr = new QDate(val);
}

/*!
    Constructs a new variant with a time value, \a val.
*/
QVariant::QVariant(const QTime &val)
{
    d = startConstruction();
    d->type = Time;
    d->value.ptr = new QTime(val);
}

/*!
    Constructs a new variant with a date/time value, \a val.
*/
QVariant::QVariant(const QDateTime &val)
{
    d = startConstruction();
    d->type = DateTime;
    d->value.ptr = new QDateTime(val);
}

/*!
    Constructs a new variant with a bytearray value, \a val.
*/
QVariant::QVariant(const QByteArray &val)
{
    d = startConstruction();
    d->type = ByteArray;
    d->value.ptr = new QByteArray(val);
}

/*!
    Constructs a new variant with a bitarray value, \a val.
*/
QVariant::QVariant(const QBitArray &val)
{
    d = startConstruction();
    d->type = BitArray;
    d->value.ptr = new QBitArray(val);
}

#ifndef QT_NO_ACCEL

/*!
    Constructs a new variant with a key sequence value, \a val.
*/
QVariant::QVariant(const QKeySequence &val)
{
    d = startConstruction();
    d->type = KeySequence;
    d->value.ptr = new QKeySequence(val);
    d->is_null = false;
}

#endif

/*!
    Constructs a new variant with a pen value, \a val.
*/
QVariant::QVariant(const QPen &val)
{
    d = startConstruction();
    d->type = Pen;
    d->value.ptr = new QPen(val);
}

/*!
    Constructs a new variant with an integer value, \a val.
*/
QVariant::QVariant(int val)
{
    d = startConstruction();
    d->type = Int;
    d->value.i = val;
    d->is_null = false;
}

/*!
    Constructs a new variant with an unsigned integer value, \a val.
*/
QVariant::QVariant(uint val)
{
    d = startConstruction();
    d->type = UInt;
    d->value.u = val;
    d->is_null = false;
}

/*!
    Constructs a new variant with a long long integer value, \a val.
*/
QVariant::QVariant(Q_LLONG val)
{
    d = startConstruction();
    d->type = LongLong;
    d->value.ll = val;
    d->is_null = false;
}

/*!
    Constructs a new variant with an unsigned long long integer value, \a val.
*/

QVariant::QVariant(Q_ULLONG val)
{
    d = startConstruction();
    d->type = ULongLong;
    d->value.ull = val;
    d->is_null = false;
}

/*!
    Constructs a new variant with a boolean value, \a val. The integer
    argument is a dummy, necessary for compatibility with some
    compilers.
*/
QVariant::QVariant(bool val, int)
{
    d = startConstruction();
    d->type = Bool;
    d->value.b = val;
    d->is_null = false;
}


/*!
    Constructs a new variant with a floating point value, \a val.
*/
QVariant::QVariant(double val)
{
    d = startConstruction();
    d->type = Double;
    d->value.d = val;
    d->is_null = false;
}

#ifndef QT_NO_TEMPLATE_VARIANT
/*!
    Constructs a new variant with a list value, \a val.
*/
QVariant::QVariant(const QList<QVariant> &val)
{
    d = startConstruction();
    d->type = List;
    d->value.ptr = new QList<QVariant>(val);
    d->is_null = false;
}
#endif

/*!
    Constructs a new variant with a size policy value, \a val.
*/
QVariant::QVariant(QSizePolicy val)
{
    d = startConstruction();
    d->type = SizePolicy;
    d->value.ptr = new QSizePolicy(val);
    d->is_null = false;
}

/*!
    Assigns the value of the variant \a variant to this variant.

    This is a deep copy of the variant, but note that if the variant
    holds an explicitly shared type such as QImage, a shallow copy is
    performed.
*/
QVariant& QVariant::operator=(const QVariant &variant)
{
    Private *x = variant.d;
    ++x->ref;
    x = qAtomicSetPtr(&d, x);
    if (!--x->ref)
	cleanUp(x);
    return *this;
}

/*!
    \internal
*/
void QVariant::detach_helper()
{
    Private *x = constructPrivate(d->type, data());
    x->is_null = d->is_null;
    x = qAtomicSetPtr(&d, x);
    if (!--x->ref)
	cleanUp(x);
}

/*!
    Returns the name of the type stored in the variant. The returned
    strings describe the C++ datatype used to store the data: for
    example, "QFont", "QString", or "QList<QVariant>". An Invalid
    variant returns 0.
*/
const char *QVariant::typeName() const
{
    return typeToName(d->type);
}

/*!
    Convert this variant to type Invalid and free up any resources
    used.
*/
void QVariant::clear()
{
    if (d->ref != 1) {
	if (!--d->ref)
	    cleanUp(d);
	d = &shared_invalid;
	return;
    }
    d->clear();
}

/* Attention!

   For dependency reasons, this table is duplicated in moc.y. If you
   change one, change both.

   (Search for the word 'Attention' in moc.y.)
*/
static const int ntypes = 34;
static const char* const type_map[ntypes] =
{
    0,
    "QMap<QString,QVariant>",
    "QList<QVariant>",
    "QString",
    "QStringList",
    "QFont",
    "QPixmap",
    "QBrush",
    "QRect",
    "QSize",
    "QColor",
    "QPalette",
#ifndef QT_NO_COMPAT
    "QColorGroup",
#endif
    "QIconSet",
    "QPoint",
    "QImage",
    "int",
    "uint",
    "bool",
    "double",
    "QPointArray",
    "QRegion",
    "QBitmap",
    "QCursor",
    "QSizePolicy",
    "QDate",
    "QTime",
    "QDateTime",
    "QByteArray",
    "QBitArray",
    "QKeySequence",
    "QPen",
    "Q_LLONG",
    "Q_ULLONG"
};


/*!
    Converts the enum representation of the storage type, \a typ, to
    its string representation.
*/
const char *QVariant::typeToName(Type typ)
{
    if (typ >= ntypes)
	return 0;
    return type_map[typ];
}


/*!
    Converts the string representation of the storage type gven in \a
    name, to its enum representation.

    If the string representation cannot be converted to any enum
    representation, the variant is set to \c Invalid.
*/
QVariant::Type QVariant::nameToType(const char *name)
{
    if (name) {
	if (strcmp(name, "QCString") == 0)
	    return ByteArray;
	for (int i = 1; i < ntypes; ++i) {
	    if (strcmp(type_map[i], name) == 0)
		return (Type)i;
	}
    }
    return Invalid;
}

#ifndef QT_NO_DATASTREAM
/*!
    Internal function for loading a variant from stream \a s. Use the
    stream operators instead.

    \internal
*/
void QVariant::load(QDataStream &s)
{
    if (d == &shared_invalid)
	detach_helper();
    Q_UINT32 u;
    s >> u;
    Type t = (Type)u;

    switch (t) {
    case Invalid: {
	// Since we wrote something, we should read something
	QString x;
	s >> x;
	d->type = t;
	d->is_null = true;
    }
	break;
#ifndef QT_NO_TEMPLATE_VARIANT
    case Map: {
	QMap<QString, QVariant> *x = new QMap<QString, QVariant>;
	s >> *x;
	d->value.ptr = x;
	d->is_null = false;
    }
	break;
    case List: {
	QList<QVariant> *x = new QList<QVariant>;
	s >> *x;
	d->value.ptr = x;
	d->is_null = false;
    }
	break;
#endif
    case Cursor: {
#ifndef QT_NO_CURSOR
	QCursor *x = new QCursor;
	s >> *x;
	d->value.ptr = x;
	d->is_null = false;
#endif
    }
	break;
    case Bitmap: {
	QBitmap *x = new QBitmap;
#ifndef QT_NO_IMAGEIO
	s >> *x;
#endif
	d->value.ptr = x;
    }
	break;
    case Region: {
	QRegion *x = new QRegion;
	s >> *x;
	d->value.ptr = x;
    }
	break;
    case PointArray: {
	QPointArray *x = new QPointArray;
	s >> *x;
	d->value.ptr = x;
    }
	break;
    case String: {
	QString *x = new QString;
	s >> *x;
	d->value.ptr = x;
    }
	break;
#ifndef QT_NO_STRINGLIST
    case StringList: {
	QStringList *x = new QStringList;
	s >> *x;
	d->value.ptr = x;
	d->is_null = false;
    }
	break;
#endif // QT_NO_STRINGLIST
    case Font: {
	QFont *x = new QFont;
	s >> *x;
	d->value.ptr = x;
	d->is_null = false;
    }
	break;
    case Pixmap: {
	QPixmap *x = new QPixmap;
#ifndef QT_NO_IMAGEIO
	s >> *x;
#endif
	d->value.ptr = x;
    }
	break;
    case Image: {
	QImage *x = new QImage;
#ifndef QT_NO_IMAGEIO
	s >> *x;
#endif
	d->value.ptr = x;
    }
	break;
    case Brush: {
	QBrush *x = new QBrush;
	s >> *x;
	d->value.ptr = x;
	d->is_null = false;
    }
	break;
    case Rect: {
	QRect *x = new QRect;
	s >> *x;
	d->value.ptr = x;
    }
	break;
    case Point: {
	QPoint *x = new QPoint;
	s >> *x;
	d->value.ptr = x;
    }
	break;
    case Size: {
	QSize *x = new QSize;
	s >> *x;
	d->value.ptr = x;
    }
	break;
    case Color: {
	QColor *x = new QColor;
	s >> *x;
	d->value.ptr = x;
	d->is_null = false;
    }
	break;
#ifndef QT_NO_PALETTE
    case Palette: {
	QPalette *x = new QPalette;
	s >> *x;
	d->value.ptr = x;
	d->is_null = false;
    }
	break;
#ifndef QT_NO_COMPAT
    case ColorGroup: {
	QColorGroup *x = new QColorGroup;
	s >> *x;
	d->value.ptr = x;
	d->is_null = false;
    }
	break;
#endif
#endif
#ifndef QT_NO_ICONSET
    case IconSet: {
	QPixmap x;
	s >> x;
	d->value.ptr = new QIconSet(x);
    }
	break;
#endif
    case Int: {
	int x;
	s >> x;
	d->value.i = x;
	d->is_null = false;
    }
	break;
    case UInt: {
	uint x;
	s >> x;
	d->value.u = x;
	d->is_null = false;
    }
	break;
    case LongLong: {
	Q_LLONG x;
	s >> x;
	d->value.ll = x;
    }
	break;
    case ULongLong: {
	Q_ULLONG x;
	s >> x;
	d->value.ull = x;
    }
	break;
    case Bool: {
	Q_INT8 x;
	s >> x;
	d->value.b = x;
	d->is_null = false;
    }
	break;
    case Double: {
	double x;
	s >> x;
	d->value.d = x;
	d->is_null = false;
    }
	break;
    case SizePolicy: {
	int h, v;
	Q_INT8 hfw;
	s >> h >> v >> hfw;
	d->value.ptr = new QSizePolicy((QSizePolicy::SizeType)h, (QSizePolicy::SizeType)v,
					(bool)hfw);
	d->is_null = false;
    }
	break;
    case Date: {
	QDate *x = new QDate;
	s >> *x;
	d->value.ptr = x;
    }
	break;
    case Time: {
	QTime *x = new QTime;
	s >> *x;
	d->value.ptr = x;
    }
	break;
    case DateTime: {
	QDateTime *x = new QDateTime;
	s >> *x;
	d->value.ptr = x;
    }
	break;
    case ByteArray: {
	QByteArray *x = new QByteArray;
	s >> *x;
	d->value.ptr = x;
    }
	break;
    case BitArray: {
	QBitArray *x = new QBitArray;
	s >> *x;
	d->value.ptr = x;
    }
	break;
#ifndef QT_NO_ACCEL
    case KeySequence: {
	QKeySequence *x = new QKeySequence;
	s >> *x;
	d->value.ptr = x;
	d->is_null = false;
    }
	break;
#endif // QT_NO_ACCEL
    case Pen: {
	QPen *x = new QPen;
	s >> *x;
	d->value.ptr = x;
	d->is_null = false;
    }
	break;
    }
    d->type = t;
}

/*!
    Internal function for saving a variant to the stream \a s. Use the
    stream operators instead.

    \internal
*/
void QVariant::save(QDataStream &s) const
{
    s << (Q_UINT32)type();

    switch (d->type) {
    case Cursor:
	s << *static_cast<QCursor *>(d->value.ptr);
	break;
    case Bitmap:
#ifndef QT_NO_IMAGEIO
	s << *static_cast<QBitmap *>(d->value.ptr);
#endif
	break;
    case PointArray:
	s << *static_cast<QPointArray *>(d->value.ptr);
	break;
    case Region:
	s << *static_cast<QRegion *>(d->value.ptr);
	break;
#ifndef QT_NO_TEMPLATE_VARIANT
    case List:
	s << *static_cast<QList<QVariant> *>(d->value.ptr);
	break;
    case Map:
	s << *static_cast<QMap<QString,QVariant> *>(d->value.ptr);
	break;
#endif
    case String:
	s << *static_cast<QString *>(d->value.ptr);
	break;
#ifndef QT_NO_STRINGLIST
    case StringList:
	s << *static_cast<QStringList *>(d->value.ptr);
	break;
#endif
    case Font:
	s << *static_cast<QFont *>(d->value.ptr);
	break;
    case Pixmap:
#ifndef QT_NO_IMAGEIO
	s << *static_cast<QPixmap *>(d->value.ptr);
#endif
	break;
    case Image:
#ifndef QT_NO_IMAGEIO
	s << *static_cast<QImage *>(d->value.ptr);
#endif
	break;
    case Brush:
	s << *static_cast<QBrush *>(d->value.ptr);
	break;
    case Point:
	s << *static_cast<QPoint *>(d->value.ptr);
	break;
    case Rect:
	s << *static_cast<QRect *>(d->value.ptr);
	break;
    case Size:
	s << *static_cast<QSize *>(d->value.ptr);
	break;
    case Color:
	s << *static_cast<QColor *>(d->value.ptr);
	break;
#ifndef QT_NO_PALETTE
    case Palette:
	s << *static_cast<QPalette *>(d->value.ptr);
	break;
#ifndef QT_NO_COMPAT
    case ColorGroup:
	s << *static_cast<QColorGroup *>(d->value.ptr);
	break;
#endif
#endif
#ifndef QT_NO_ICONSET
    case IconSet:
	//### add stream operator to iconset
	s << static_cast<QIconSet *>(d->value.ptr)->pixmap();
	break;
#endif
    case Int:
	s << d->value.i;
	break;
    case UInt:
	s << d->value.u;
	break;
    case LongLong:
	s << d->value.ll;
	break;
    case ULongLong:
	s << d->value.ull;
	break;
    case Bool:
	s << (Q_INT8)d->value.b;
	break;
    case Double:
	s << d->value.d;
	break;
    case SizePolicy:
	{
	    QSizePolicy p = toSizePolicy();
	    s << (int) p.horData() << (int) p.verData()
	      << (Q_INT8) p.hasHeightForWidth();
	}
	break;
    case Date:
	s << *static_cast<QDate *>(d->value.ptr);
	break;
    case Time:
	s << *static_cast<QTime *>(d->value.ptr);
	break;
    case DateTime:
	s << *static_cast<QDateTime *>(d->value.ptr);
	break;
    case ByteArray:
	s << *static_cast<QByteArray *>(d->value.ptr);
	break;
    case BitArray:
	s << *static_cast<QBitArray *>(d->value.ptr);
	break;
#ifndef QT_NO_ACCEL
    case KeySequence:
	s << *static_cast<QKeySequence *>(d->value.ptr);
	break;
#endif
    case Pen:
	s << *static_cast<QPen *>(d->value.ptr);
	break;
    case Invalid:
	s << QString();
	break;
    }
}

/*!
    Reads a variant \a p from the stream \a s.

    \sa \link datastreamformat.html Format of the QDataStream
    operators \endlink
*/
QDataStream& operator>>(QDataStream &s, QVariant &p)
{
    p.load(s);
    return s;
}

/*!
    Writes a variant \a p to the stream \a s.

    \sa \link datastreamformat.html Format of the QDataStream
    operators \endlink
*/
QDataStream& operator<<(QDataStream &s, const QVariant &p)
{
    p.save(s);
    return s;
}

/*!
    Reads a variant type \a p in enum representation from the stream \a s.
*/
QDataStream& operator>>(QDataStream &s, QVariant::Type &p)
{
    Q_UINT32 u;
    s >> u;
    p = (QVariant::Type)u;

    return s;
}

/*!
    Writes a variant type \a p to the stream \a s.
*/
QDataStream& operator<<(QDataStream &s, const QVariant::Type p)
{
    s << (Q_UINT32)p;

    return s;
}

#endif //QT_NO_DATASTREAM

/*!
    \fn Type QVariant::type() const

    Returns the storage type of the value stored in the variant.
    Usually it's best to test with canCast() whether the variant can
    deliver the data type you are interested in.
*/

/*!
    \fn bool QVariant::isValid() const

    Returns true if the storage type of this variant is not
    QVariant::Invalid; otherwise returns false.
*/

/*! \fn QByteArray QVariant::toCString() const
  \obsolete
    Returns the variant as a QCString if the variant has type()
    CString or String; otherwise returns 0.

    \sa asCString()
*/

/*!
    Returns the variant as a QString if the variant has type() String,
    ByteArray, Int, Uint, Bool, Double, Date, Time, DateTime,
    KeySequence, Font or Color; otherwise returns QString::null.

    \sa asString()
*/
QString QVariant::toString() const
{
    switch (d->type) {
    case Int:
	return QString::number(toInt());
    case UInt:
	return QString::number(toUInt());
    case LongLong:
	return QString::number(toLongLong());
    case ULongLong:
	return QString::number(toULongLong());
    case Double:
	return QString::number(toDouble(), 'g', DBL_DIG);
#if !defined(QT_NO_SPRINTF) && !defined(QT_NO_DATESTRING)
    case Date:
	return toDate().toString(Qt::ISODate);
    case Time:
	return toTime().toString(Qt::ISODate);
    case DateTime:
	return toDateTime().toString(Qt::ISODate);
#endif
    case Bool:
	return toInt() ? "true" : "false";
#ifndef QT_NO_ACCEL
    case KeySequence:
	return QString(*static_cast<QKeySequence *>(d->value.ptr));
#endif
    case ByteArray:
	return QString(static_cast<QByteArray *>(d->value.ptr)->constData());
    case Font:
	return toFont().toString();
    case Color:
	return toColor().name();
    case String:
	return *static_cast<QString *>(d->value.ptr);
    default:
	return QString::null;
    }
}

#ifndef QT_NO_STRINGLIST
/*!
    Returns the variant as a QStringList if the variant has type()
    StringList or List of a type that can be converted to QString;
    otherwise returns an empty list.

    Note that if you want to iterate over the list, you should iterate
    over a copy, e.g.
    \code
    QStringList list = myVariant.toStringList();
    QStringList::Iterator it = list.begin();
    while( it != list.end() ) {
	myProcessing( *it );
	++it;
    }
    \endcode

    \sa asStringList()
*/
QStringList QVariant::toStringList() const
{
    switch (d->type) {
    case StringList:
	return *static_cast<QStringList *>(d->value.ptr);
#ifndef QT_NO_TEMPLATE_VARIANT
    case List:
	{
	    QStringList slst;
	    QList<QVariant> list(toList());
	    for (int i = 0; i < list.size(); ++i)
		slst.append(list.at(i).toString());
	    return slst;
	}
#endif
    default:
	return QStringList();
    }
}
#endif //QT_NO_STRINGLIST

#ifndef QT_NO_TEMPLATE_VARIANT
/*!
    Returns the variant as a QMap<QString,QVariant> if the variant has
    type() Map; otherwise returns an empty map.

    Note that if you want to iterate over the map, you should iterate
    over a copy, e.g.
    \code
    QMap<QString, QVariant> map = myVariant.toMap();
    QMap<QString, QVariant>::Iterator it = map.begin();
    while( it != map.end() ) {
	myProcessing( *it );
	++it;
    }
    \endcode

    \sa asMap()
*/
QMap<QString, QVariant> QVariant::toMap() const
{
    if (d->type != Map)
	return QMap<QString,QVariant>();

    return *static_cast<QMap<QString, QVariant> *>(d->value.ptr);
}
#endif
/*!
    Returns the variant as a QFont if the variant has type() Font;
    otherwise returns the application's default font.

  \sa asFont()
*/
QFont QVariant::toFont() const
{
    switch (d->type) {
    case String:
	{
	    QFont fnt;
	    fnt.fromString(toString());
	    return fnt;
	}
    case Font:
	return *static_cast<QFont *>(d->value.ptr);
    default:
	return QFont();
    }
}

/*!
    Returns the variant as a QPixmap if the variant has type() Pixmap;
    otherwise returns a null pixmap.

    \sa asPixmap()
*/
QPixmap QVariant::toPixmap() const
{
    if (d->type != Pixmap)
	return QPixmap();

    return *static_cast<QPixmap *>(d->value.ptr);
}

/*!
    Returns the variant as a QImage if the variant has type() Image;
    otherwise returns a null image.

    \sa asImage()
*/
const QImage QVariant::toImage() const
{
    if (d->type != Image)
	return QImage();

    return *static_cast<QImage *>(d->value.ptr);
}

/*!
    Returns the variant as a QBrush if the variant has type() Brush;
    otherwise returns a default brush (with all black colors).

    \sa asBrush()
*/
QBrush QVariant::toBrush() const
{
    if (d->type != Brush)
	return QBrush();

    return *static_cast<QBrush *>(d->value.ptr);
}

/*!
    Returns the variant as a QPoint if the variant has type() Point;
    otherwise returns a point (0, 0).

    \sa asPoint()
*/
QPoint QVariant::toPoint() const
{
    if (d->type != Point)
	return QPoint();

    return *static_cast<QPoint *>(d->value.ptr);
}

/*!
    Returns the variant as a QRect if the variant has type() Rect;
    otherwise returns an empty rectangle.

    \sa asRect()
*/
QRect QVariant::toRect() const
{
    if (d->type != Rect)
	return QRect();

    return *static_cast<QRect *>(d->value.ptr);
}

/*!
    Returns the variant as a QSize if the variant has type() Size;
    otherwise returns an invalid size.

    \sa asSize()
*/
QSize QVariant::toSize() const
{
    if (d->type != Size)
	return QSize();

    return *static_cast<QSize *>(d->value.ptr);
}

/*!
    Returns the variant as a QColor if the variant has type() Color;
    otherwise returns an invalid color.

    \sa asColor()
*/
QColor QVariant::toColor() const
{
    switch (d->type) {
    case String:
	{
	    QColor col;
	    col.setNamedColor(toString());
	    return col;
	}
    case Color:
	return *static_cast<QColor *>(d->value.ptr);
    default:
	return QColor();
    }
}
#ifndef QT_NO_PALETTE
/*!
    Returns the variant as a QPalette if the variant has type()
    Palette; otherwise returns a completely black palette.

    \sa asPalette()
*/
QPalette QVariant::toPalette() const
{
    if (d->type != Palette)
	return QPalette();

    return *static_cast<QPalette *>(d->value.ptr);
}

#ifndef QT_NO_COMPAT
QColorGroup QVariant::toColorGroup() const
{
    if (d->type != ColorGroup)
	return QColorGroup();
    return *static_cast<QColorGroup *>(d->value.ptr);
}
#endif
#endif //QT_NO_PALETTE
#ifndef QT_NO_ICONSET
/*!
    Returns the variant as a QIconSet if the variant has type()
    IconSet; otherwise returns an icon set of null pixmaps.

    \sa asIconSet()
*/
QIconSet QVariant::toIconSet() const
{
    if (d->type != IconSet)
	return QIconSet();

    return *static_cast<QIconSet *>(d->value.ptr);
}
#endif //QT_NO_ICONSET
/*!
    Returns the variant as a QPointArray if the variant has type()
    PointArray; otherwise returns an empty QPointArray.

    \sa asPointArray()
*/
const QPointArray QVariant::toPointArray() const
{
    if (d->type != PointArray)
	return QPointArray();

    return *static_cast<QPointArray *>(d->value.ptr);
}

/*!
    Returns the variant as a QBitmap if the variant has type() Bitmap;
    otherwise returns a null QBitmap.

    \sa asBitmap()
*/
QBitmap QVariant::toBitmap() const
{
    if (d->type != Bitmap)
	return QBitmap();

    return *static_cast<QBitmap *>(d->value.ptr);
}

/*!
    Returns the variant as a QRegion if the variant has type() Region;
    otherwise returns an empty QRegion.

    \sa asRegion()
*/
QRegion QVariant::toRegion() const
{
    if (d->type != Region)
	return QRegion();

    return *static_cast<QRegion *>(d->value.ptr);
}

/*!
    Returns the variant as a QCursor if the variant has type() Cursor;
    otherwise returns the default arrow cursor.

    \sa asCursor()
*/
QCursor QVariant::toCursor() const
{
#ifndef QT_NO_CURSOR
    if (d->type != Cursor)
	return QCursor();
#endif

    return *static_cast<QCursor *>(d->value.ptr);
}

/*!
    Returns the variant as a QDate if the variant has type() Date,
    DateTime or String; otherwise returns an invalid date.

    Note that if the type() is String an invalid date will be returned
    if the string cannot be parsed as a Qt::ISODate format date.

    \sa asDate()
*/
QDate QVariant::toDate() const
{
    switch (d->type) {
    case Date:
	return *static_cast<QDate *>(d->value.ptr);
    case DateTime:
	return ((QDateTime*)d->value.ptr)->date();
#ifndef QT_NO_DATESTRING
    case String:
	return QDate::fromString( *((QString*)d->value.ptr), Qt::ISODate );
#endif
    default:
	return QDate();
    }
}

/*!
    Returns the variant as a QTime if the variant has type() Time,
    DateTime or String; otherwise returns an invalid time.

    Note that if the type() is String an invalid time will be returned
    if the string cannot be parsed as a Qt::ISODate format time.

    \sa asTime()
*/
QTime QVariant::toTime() const
{
    switch ( d->type ) {
    case Time:
	return *((QTime*)d->value.ptr);
    case DateTime:
	return ((QDateTime*)d->value.ptr)->time();
#ifndef QT_NO_DATESTRING
    case String:
	return QTime::fromString( *((QString*)d->value.ptr), Qt::ISODate );
#endif
    default:
	return QTime();
    }
}

/*!
    Returns the variant as a QDateTime if the variant has type()
    DateTime or String; otherwise returns an invalid date/time.

    Note that if the type() is String an invalid date/time will be
    returned if the string cannot be parsed as a Qt::ISODate format
    date/time.

    \sa asDateTime()
*/
QDateTime QVariant::toDateTime() const
{
    switch ( d->type ) {
    case DateTime:
	return *static_cast<QDateTime *>(d->value.ptr);
#ifndef QT_NO_DATESTRING
    case String:
	return QDateTime::fromString(*static_cast<QString *>(d->value.ptr), Qt::ISODate);
#endif
    case Date:
	return QDateTime(*static_cast<QDate*>(d->value.ptr));
    default:
	return QDateTime();
    }
}

/*!
    Returns the variant as a QByteArray if the variant has type()
    ByteArray; otherwise returns an empty bytearray.

    \sa asByteArray()
*/
QByteArray QVariant::toByteArray() const
{
    if (d->type == ByteArray)
	return *static_cast<QByteArray *>(d->value.ptr);
    else if (d->type == String)
	return static_cast<QString *>(d->value.ptr)->toAscii();
    return QByteArray();
}

/*!
    Returns the variant as a QBitArray if the variant has type()
    BitArray; otherwise returns an empty bitarray.

    \sa asBitArray()
*/
QBitArray QVariant::toBitArray() const
{
    if (d->type == BitArray)
	return *static_cast<QBitArray *>(d->value.ptr);
    return QBitArray();
}

#ifndef QT_NO_ACCEL

/*!
    Returns the variant as a QKeySequence if the variant has type()
    KeySequence, Int or String; otherwise returns an empty key
    sequence.

    Note that not all Ints and Strings are valid key sequences and in
    such cases an empty key sequence will be returned.

    \sa asKeySequence()
*/
QKeySequence QVariant::toKeySequence() const
{
    switch (d->type) {
    case KeySequence:
	return *static_cast<QKeySequence*>(d->value.ptr);
    case String:
	return QKeySequence(toString());
    case Int:
	return QKeySequence(toInt());
    default:
	return QKeySequence();
    }
}

#endif // QT_NO_ACCEL

/*!
    Returns the variant as a QPen if the variant has type()
    Pen; otherwise returns an empty QPen.

    \sa asPen()
*/
QPen QVariant::toPen() const
{
    if (d->type != Pen)
	return QPen();

    return *static_cast<QPen*>(d->value.ptr);
}

/*!
    Returns the variant as an int if the variant has type() String,
    Int, UInt, Double, Bool or KeySequence; otherwise returns
    0.

    If \a ok is non-null: \a *ok is set to true if the value could be
    converted to an int; otherwise \a *ok is set to false.

    \sa asInt() canCast()
*/
int QVariant::toInt(bool *ok) const
{
    if (ok)
	*ok = canCast(Int);

    switch (d->type) {
    case String:
	return static_cast<QString *>(d->value.ptr)->toInt(ok);
    case ByteArray:
	return QString(*static_cast<QByteArray *>(d->value.ptr)).toInt(ok);
    case Int:
	return d->value.i;
    case UInt:
	return (int)d->value.u;
    case LongLong:
        return (int)d->value.ll;
    case ULongLong:
        return (int)d->value.ull;
    case Double:
	return (int)d->value.d;
    case Bool:
	return (int)d->value.b;
#ifndef QT_NO_ACCEL
    case KeySequence:
	return (int)*( (QKeySequence*)d->value.ptr );
#endif
    default:
	return 0;
    }
}

/*!
    Returns the variant as an unsigned int if the variant has type()
    String, ByteArray, UInt, Int, Double, or Bool; otherwise returns 0.

    If \a ok is non-null: \a *ok is set to true if the value could be
    converted to an unsigned int; otherwise \a *ok is set to false.

    \sa asUInt()
*/
uint QVariant::toUInt(bool *ok) const
{
    if (ok)
	*ok = canCast(UInt);

    switch (d->type) {
    case String:
	return static_cast<QString *>(d->value.ptr)->toUInt(ok);
    case ByteArray:
	return QString(*static_cast<QByteArray *>(d->value.ptr)).toUInt(ok);
    case Int:
	return (uint)d->value.i;
    case UInt:
	return d->value.u;
    case LongLong:
        return (uint)d->value.ll;
    case ULongLong:
        return (uint)d->value.ull;
    case Double:
	return (uint)d->value.d;
    case Bool:
	return (uint)d->value.b;
    default:
	return 0;
    }
}

/*!
    Returns the variant as a long long int if the variant has type()
    LongLong, ULongLong, any type allowing a toInt() conversion;
    otherwise returns 0.

    If \a ok is non-null: \a *ok is set to true if the value could be
    converted to an int; otherwise \a *ok is set to false.

    \sa asLongLong() canCast()
*/
Q_LLONG QVariant::toLongLong(bool *ok) const
{
    if (ok)
	*ok = canCast(LongLong);

    switch (d->type) {
    case String:
	return static_cast<QString *>(d->value.ptr)->toLongLong(ok);
    case ByteArray:
	return QString(*static_cast<QByteArray *>(d->value.ptr)).toLongLong(ok);
    case Int:
	return (Q_LLONG)d->value.i;
    case UInt:
	return (Q_LLONG)d->value.u;
    case LongLong:
	return d->value.ll;
    case ULongLong:
	return (Q_LLONG)d->value.ull;
    case Double:
	return (Q_LLONG)d->value.d;
    case Bool:
	return (Q_LLONG)d->value.b;
    default:
	return 0;
    }
}

/*!
    Returns the variant as as an unsigned long long int if the variant
    has type() LongLong, ULongLong, any type allowing a toUInt()
    conversion; otherwise returns 0.

    If \a ok is non-null: \a *ok is set to true if the value could be
    converted to an int; otherwise \a *ok is set to false.

    \sa asULongLong() canCast()
*/
Q_ULLONG QVariant::toULongLong(bool *ok) const
{
    if (ok)
	*ok = canCast(ULongLong);

    switch (d->type) {
    case Int:
	return (Q_ULLONG)d->value.i;
    case UInt:
	return (Q_ULLONG)d->value.u;
    case LongLong:
	return (Q_ULLONG)d->value.ll;
    case ULongLong:
	return d->value.ull;
    case Double:
	return (Q_ULLONG)d->value.d;
    case Bool:
	return (Q_ULLONG)d->value.b;
    case String:
	return static_cast<QString *>(d->value.ptr)->toULongLong(ok);
    case ByteArray:
	return QString(*static_cast<QByteArray *>(d->value.ptr)).toULongLong(ok);
    default:
	return 0;
    }
}

/*!
    Returns the variant as a bool if the variant has type() Bool.

    Returns true if the variant has type Int, UInt or Double and its
    value is non-zero, or if the variant has type String and its lower-case
    content is not empty, "0" or "false"; otherwise returns false.

    \sa asBool()
*/
bool QVariant::toBool() const
{
    switch(d->type) {
    case Bool:
	return d->value.b;
    case Double:
	return d->value.d != 0.0;
    case Int:
	return d->value.i != 0;
    case UInt:
	return d->value.u != 0;
    case LongLong:
	return d->value.ll != 0;
    case ULongLong:
	return d->value.ull != 0;
    case String:
	{
	    QString str = toString().lower();
	    return !(str == "0" || str == "false" || str.isEmpty());
	}
    default:
	return false;
    }
}

/*!
    Returns the variant as a double if the variant has type() String,
    ByteArray, Double, Int, UInt, LongLong, ULongLong or Bool; otherwise
    returns 0.0.

    If \a ok is non-null: \a *ok is set to true if the value could be
    converted to a double; otherwise \a *ok is set to false.

    \sa asDouble()
*/
double QVariant::toDouble(bool *ok) const
{
    if (ok)
	*ok = canCast(Double);

    switch (d->type) {
    case String:
	return static_cast<QString *>(d->value.ptr)->toDouble(ok);
    case ByteArray:
	return QString(*static_cast<QByteArray *>(d->value.ptr)).toDouble(ok);
    case Double:
	return d->value.d;
    case Int:
	return (double)d->value.i;
    case Bool:
	return (double)d->value.b;
    case UInt:
	return (double)d->value.u;
    case LongLong:
	return (double)d->value.ll;
    case ULongLong:
#if defined(Q_CC_MSVC) && !defined(Q_CC_MSVC_NET)
	return (double)(Q_LLONG)d->value.ull;
#else
	return (double)d->value.ull;
#endif
    default:
	return 0.0;
    }
}

#ifndef QT_NO_TEMPLATE_VARIANT
/*!
    Returns the variant as a QList<QVariant> if the variant has
    type() List or StringList; otherwise returns an empty list.

    Note that if you want to iterate over the list, you should iterate
    over a copy, e.g.
    \code
    QList<QVariant> list = myVariant.toList();
    QList<QVariant>::Iterator it = list.begin();
    while( it != list.end() ) {
	myProcessing( *it );
	++it;
    }
    \endcode

    \sa asList()
*/
QList<QVariant> QVariant::toList() const
{
    if (d->type == List)
	return *static_cast<QList<QVariant> *>(d->value.ptr);
#ifndef QT_NO_STRINGLIST
    if (d->type == StringList) {
	QList<QVariant> lst;
	QStringList slist(toStringList());
	for (int i = 0; i < slist.size(); ++i)
	    lst.append(QVariant(slist.at(i)));
	return lst;
    }
#endif //QT_NO_STRINGLIST
    return QList<QVariant>();
}
#endif

/*!
    Returns the variant as a QSizePolicy if the variant has type()
    SizePolicy; otherwise returns an undefined (but legal) size
    policy.
*/

QSizePolicy QVariant::toSizePolicy() const
{
    if (d->type == SizePolicy)
	return *static_cast<QSizePolicy *>(d->value.ptr);

    return QSizePolicy();
}


#define Q_VARIANT_AS( f ) Q##f& QVariant::as##f() { \
   if ( d->type != f ) *this = QVariant( to##f() ); else detach(); return *static_cast<Q##f*>(d->value.ptr);}

Q_VARIANT_AS(String)
#ifndef QT_NO_STRINGLIST
Q_VARIANT_AS(StringList)
#endif
Q_VARIANT_AS(Font)
Q_VARIANT_AS(Pixmap)
Q_VARIANT_AS(Image)
Q_VARIANT_AS(Brush)
Q_VARIANT_AS(Point)
Q_VARIANT_AS(Rect)
Q_VARIANT_AS(Size)
Q_VARIANT_AS(Color)
#ifndef QT_NO_PALETTE
Q_VARIANT_AS(Palette)
#ifndef QT_NO_COMPAT
Q_VARIANT_AS(ColorGroup)
#endif
#endif
#ifndef QT_NO_ICONSET
Q_VARIANT_AS(IconSet)
#endif
Q_VARIANT_AS(PointArray)
Q_VARIANT_AS(Bitmap)
Q_VARIANT_AS(Region)
Q_VARIANT_AS(Cursor)
Q_VARIANT_AS(SizePolicy)
Q_VARIANT_AS(Date)
Q_VARIANT_AS(Time)
Q_VARIANT_AS(DateTime)
Q_VARIANT_AS(ByteArray)
Q_VARIANT_AS(BitArray)
#ifndef QT_NO_ACCEL
Q_VARIANT_AS(KeySequence)
#endif
Q_VARIANT_AS(Pen)

/*!
    \fn QString& QVariant::asString()

    Tries to convert the variant to hold a string value. If that is
    not possible the variant is set to an empty string.

    Returns a reference to the stored string.

    \sa toString()
*/

/*!
    \fn QCString& QVariant::asCString()

    \obsolete

    Tries to convert the variant to hold a string value. If that is
    not possible the variant is set to an empty string.

    Returns a reference to the stored string.

    \sa toCString()
*/

/*!
    \fn QStringList& QVariant::asStringList()

    Tries to convert the variant to hold a QStringList value. If that
    is not possible the variant is set to an empty string list.

    Returns a reference to the stored string list.

    Note that if you want to iterate over the list, you should
    iterate over a copy, e.g.
    \code
    QStringList list = myVariant.asStringList();
    QStringList::Iterator it = list.begin();
    while( it != list.end() ) {
	myProcessing( *it );
	++it;
    }
    \endcode

    \sa toStringList()
*/

/*!
    \fn QFont& QVariant::asFont()

    Tries to convert the variant to hold a QFont. If that is not
    possible the variant is set to the application's default font.

    Returns a reference to the stored font.

    \sa toFont()
*/

/*!
    \fn QPixmap& QVariant::asPixmap()

    Tries to convert the variant to hold a pixmap value. If that is
    not possible the variant is set to a null pixmap.

    Returns a reference to the stored pixmap.

    \sa toPixmap()
*/

/*!
    \fn QImage& QVariant::asImage()

    Tries to convert the variant to hold an image value. If that is
    not possible the variant is set to a null image.

    Returns a reference to the stored image.

    \sa toImage()
*/

/*!
    \fn QBrush& QVariant::asBrush()

    Tries to convert the variant to hold a brush value. If that is not
    possible the variant is set to a default black brush.

    Returns a reference to the stored brush.

    \sa toBrush()
*/

/*!
    \fn QPoint& QVariant::asPoint()

    Tries to convert the variant to hold a point value. If that is not
    possible the variant is set to a (0, 0) point.

    Returns a reference to the stored point.

    \sa toPoint()
*/

/*!
    \fn QRect& QVariant::asRect()

    Tries to convert the variant to hold a rectangle value. If that is
    not possible the variant is set to an empty rectangle.

    Returns a reference to the stored rectangle.

    \sa toRect()
*/

/*!
    \fn QSize& QVariant::asSize()

    Tries to convert the variant to hold a QSize value. If that is not
    possible the variant is set to an invalid size.

    Returns a reference to the stored size.

    \sa toSize() QSize::isValid()
*/

/*!
    \fn QSizePolicy& QVariant::asSizePolicy()

    Tries to convert the variant to hold a QSizePolicy value. If that
    fails, the variant is set to an arbitrary (valid) size policy.
*/


/*!
    \fn QColor& QVariant::asColor()

    Tries to convert the variant to hold a QColor value. If that is
    not possible the variant is set to an invalid color.

    Returns a reference to the stored color.

    \sa toColor() QColor::isValid()
*/

/*!
    \fn QPalette& QVariant::asPalette()

    Tries to convert the variant to hold a QPalette value. If that is
    not possible the variant is set to a palette of black colors.

    Returns a reference to the stored palette.

    \sa toString()
*/

/*!
    \fn QIconSet& QVariant::asIconSet()

    Tries to convert the variant to hold a QIconSet value. If that is
    not possible the variant is set to an empty iconset.

    Returns a reference to the stored iconset.

    \sa toIconSet()
*/

/*!
    \fn QPointArray& QVariant::asPointArray()

    Tries to convert the variant to hold a QPointArray value. If that
    is not possible the variant is set to an empty point array.

    Returns a reference to the stored point array.

    \sa toPointArray()
*/

/*!
    \fn QBitmap& QVariant::asBitmap()

    Tries to convert the variant to hold a bitmap value. If that is
    not possible the variant is set to a null bitmap.

    Returns a reference to the stored bitmap.

    \sa toBitmap()
*/

/*!
    \fn QRegion& QVariant::asRegion()

    Tries to convert the variant to hold a QRegion value. If that is
    not possible the variant is set to a null region.

    Returns a reference to the stored region.

    \sa toRegion()
*/

/*!
    \fn QCursor& QVariant::asCursor()

    Tries to convert the variant to hold a QCursor value. If that is
    not possible the variant is set to a default arrow cursor.

    Returns a reference to the stored cursor.

    \sa toCursor()
*/

/*!
    \fn QDate& QVariant::asDate()

    Tries to convert the variant to hold a QDate value. If that is not
    possible then the variant is set to an invalid date.

    Returns a reference to the stored date.

    \sa toDate()
*/

/*!
    \fn QTime& QVariant::asTime()

    Tries to convert the variant to hold a QTime value. If that is not
    possible then the variant is set to an invalid time.

    Returns a reference to the stored time.

    \sa toTime()
*/

/*!
    \fn QDateTime& QVariant::asDateTime()

    Tries to convert the variant to hold a QDateTime value. If that is
    not possible then the variant is set to an invalid date/time.

    Returns a reference to the stored date/time.

    \sa toDateTime()
*/

/*!
    \fn QByteArray& QVariant::asByteArray()

    Tries to convert the variant to hold a QByteArray value. If that
    is not possible then the variant is set to an empty bytearray.

    Returns a reference to the stored bytearray.

    \sa toByteArray()
*/

/*!
    \fn QBitArray& QVariant::asBitArray()

    Tries to convert the variant to hold a QBitArray value. If that is
    not possible then the variant is set to an empty bitarray.

    Returns a reference to the stored bitarray.

    \sa toBitArray()
*/

/*!
    \fn QKeySequence& QVariant::asKeySequence()

    Tries to convert the variant to hold a QKeySequence value. If that
    is not possible then the variant is set to an empty key sequence.

    Returns a reference to the stored key sequence.

    \sa toKeySequence()
*/

/*! \fn QPen& QVariant::asPen()

  Tries to convert the variant to hold a QPen value. If that
  is not possible then the variant is set to an empty pen.

  Returns a reference to the stored pen.

  \sa toPen()
*/

/*!
    Returns the variant's value as int reference.
*/
int &QVariant::asInt()
{
    if (d->type != Int) {
	detach();
	int i = toInt();
	d->clear();
	d->value.i = i;
	d->type = Int;
    }
    return d->value.i;
}

/*!
    Returns the variant's value as unsigned int reference.
*/
uint &QVariant::asUInt()
{
    if (d->type != UInt) {
	detach();
	uint u = toUInt();
	d->clear();
	d->value.u = u;
	d->type = UInt;
    }
    return d->value.u;
}

/*!
    Returns the variant's value as long long reference.
*/
Q_LLONG &QVariant::asLongLong()
{
    if (d->type != LongLong) {
	detach();
	Q_LLONG ll = toLongLong();
	d->clear();
	d->value.ll = ll;
	d->type = LongLong;
    }
    return d->value.ll;
}

/*!
    Returns the variant's value as unsigned long long reference.
*/
Q_ULLONG &QVariant::asULongLong()
{
    if (d->type != ULongLong) {
	detach();
	Q_ULLONG ull = toULongLong();
	d->clear();
	d->value.ull = ull;
	d->type = ULongLong;
    }
    return d->value.ull;
}

/*!
    Returns the variant's value as bool reference.
*/
bool &QVariant::asBool()
{
    if (d->type != Bool) {
	detach();
	bool b = toBool();
	d->clear();
	d->value.b = b;
	d->type = Bool;
    }
    return d->value.b;
}

/*!
    Returns the variant's value as double reference.
*/
double &QVariant::asDouble()
{
    if (d->type != Double) {
	detach();
	double dbl = toDouble();
	d->clear();
	d->value.d = dbl;
	d->type = Double;
    }
    return d->value.d;
}

#ifndef QT_NO_TEMPLATE_VARIANT
/*!
    Returns the variant's value as variant list reference.

    Note that if you want to iterate over the list, you should iterate
    over a copy, e.g.
    \code
    QList<QVariant> list = myVariant.asList();
    QList<QVariant>::Iterator it = list.begin();
    while( it != list.end() ) {
	myProcessing( *it );
	++it;
    }
    \endcode
*/
QList<QVariant>& QVariant::asList()
{
    if (d->type != List)
	*this = QVariant(toList());
    return *static_cast<QList<QVariant> *>(d->value.ptr);
}

/*!
    Returns the variant's value as variant map reference.

    Note that if you want to iterate over the map, you should iterate
    over a copy, e.g.
    \code
    QMap<QString, QVariant> map = myVariant.asMap();
    QMap<QString, QVariant>::Iterator it = map.begin();
    while( it != map.end() ) {
	myProcessing( *it );
	++it;
    }
    \endcode
*/
QMap<QString, QVariant>& QVariant::asMap()
{
    if (d->type != Map)
	*this = QVariant(toMap());
    return *static_cast<QMap<QString, QVariant> *>(d->value.ptr);
}
#endif

/*!
    Returns true if the variant's type can be cast to the requested
    type, \a t. Such casting is done automatically when calling the
    toInt(), toBool(), ... or asInt(), asBool(), ... methods.

    The following casts are done automatically:
    \table
    \header \i Type \i Automatically Cast To
    \row \i Bool \i Double, Int, UInt, LongLong, ULongLong
    \row \i Color \i String
    \row \i Date \i String, DateTime
    \row \i DateTime \i String, Date, Time
    \row \i Double \i String, Int, Bool, UInt
    \row \i Font \i String
    \row \i Int \i String, Double, Bool, UInt
    \row \i List \i StringList (if the list contains strings or
    something that can be cast to a string)
    \row \i String \i CString, Int, Uint, Bool, Double, Date,
    Time, DateTime, KeySequence, Font, Color
    \row \i CString \i String
    \row \i StringList \i List
    \row \i Time \i String
    \row \i UInt \i String, Double, Bool, Int
    \row \i KeySequence \i String, Int
    \endtable
*/
bool QVariant::canCast(Type t) const
{
    if (d->type == t)
	return true;
    switch ( t ) {
    case Bool:
	return d->type == Double || d->type == Int || d->type == UInt
	     || d->type == LongLong || d->type == ULongLong || d->type == String;
    case Int:
	return d->type == String || d->type == Double || d->type == Bool
	    || d->type == UInt || d->type == LongLong || d->type == ULongLong
	    || d->type == KeySequence;
    case UInt:
	return d->type == String || d->type == Double || d->type == Bool
	    || d->type == Int || d->type == LongLong || d->type == ULongLong;
    case LongLong:
	return d->type == String || d->type == Double || d->type == Bool
	    || d->type == Int || d->type == UInt || d->type == ULongLong || d->type == KeySequence;
    case ULongLong:
	return d->type == String || d->type == Double || d->type == Bool
	    || d->type == Int || d->type == UInt || d->type == LongLong;
    case Double:
	return d->type == String || d->type == Int || d->type == Bool
	    || d->type == UInt || d->type == LongLong || d->type == ULongLong;
    case String:
	return d->type == ByteArray || d->type == Int
	    || d->type == UInt || d->type == Bool || d->type == Double
	    || d->type == Date || d->type == Time || d->type == DateTime
	    || d->type == KeySequence || d->type == Font || d->type == Color
	    || d->type == LongLong || d->type == ULongLong;
    case ByteArray:
	return d->type == CString || d->type == String;
    case Date:
	return d->type == String || d->type == DateTime;
    case Time:
	return d->type == String || d->type == DateTime;
    case DateTime:
	return d->type == String || d->type == Date;
    case KeySequence:
	return d->type == String || d->type == Int;
    case Font:
	return d->type == String;
    case Color:
	return d->type == String;
#ifndef QT_NO_STRINGLIST
    case List:
	return d->type == StringList;
#endif
#ifndef QT_NO_TEMPLATE_VARIANT
    case StringList:
	if (d->type == List) {
	    const QList<QVariant> &varlist = *static_cast<QList<QVariant> *>(d->value.ptr);
	    for (int i = 0; i < varlist.size(); ++i) {
		if (!varlist.at(i).canCast(String))
		    return false;
	    }
	    return true;
	}
	return false;
#endif
    default:
	return false;
    }
}

/*!
    Casts the variant to the requested type. If the cast cannot be
    done, the variant is set to the default value of the requested
    type (e.g. an empty string if the requested type \a t is
    QVariant::String, an empty point array if the requested type \a t
    is QVariant::PointArray, etc). Returns true if the current type of
    the variant was successfully cast; otherwise returns false.

    \sa canCast()
*/

bool QVariant::cast(Type t)
{
    switch (t) {
#ifndef QT_NO_TEMPLATE_VARIANT
    case QVariant::Map:
	asMap();
	break;
    case QVariant::List:
	asList();
	break;
#endif
    case QVariant::String:
	asString();
	break;
#ifndef QT_NO_STRINGLIST
    case QVariant::StringList:
	asStringList();
	break;
#endif
    case QVariant::Font:
	asFont();
	break;
    case QVariant::Pixmap:
	asPixmap();
	break;
    case QVariant::Brush:
	asBrush();
	break;
    case QVariant::Rect:
	asRect();
	break;
    case QVariant::Size:
	asSize();
	break;
    case QVariant::Color:
	asColor();
	break;
#ifndef QT_NO_PALETTE
    case QVariant::Palette:
	asPalette();
	break;
#ifndef QT_NO_COMPAT
    case QVariant::ColorGroup:
	asColorGroup();
	break;
#endif
#endif
#ifndef QT_NO_ICONSET
    case QVariant::IconSet:
	asIconSet();
	break;
#endif
    case QVariant::Point:
	asPoint();
	break;
    case QVariant::Image:
	asImage();
	break;
    case QVariant::Int:
	asInt();
	break;
    case QVariant::UInt:
	asUInt();
	break;
    case QVariant::Bool:
	asBool();
	break;
    case QVariant::Double:
	asDouble();
	break;
    case QVariant::PointArray:
	asPointArray();
	break;
    case QVariant::Region:
	asRegion();
	break;
    case QVariant::Bitmap:
	asBitmap();
	break;
    case QVariant::Cursor:
	asCursor();
	break;
    case QVariant::SizePolicy:
	asSizePolicy();
	break;
    case QVariant::Date:
	asDate();
	break;
    case QVariant::Time:
	asTime();
	break;
    case QVariant::DateTime:
	asDateTime();
	break;
    case QVariant::ByteArray:
	asByteArray();
	break;
    case QVariant::BitArray:
	asBitArray();
	break;
#ifndef QT_NO_ACCEL
    case QVariant::KeySequence:
	asKeySequence();
	break;
#endif
    case QVariant::Pen:
	asPen();
	break;
    case QVariant::LongLong:
	asLongLong();
	break;
    case QVariant::ULongLong:
	asULongLong();
	break;
    default:
    case QVariant::Invalid:
	*this = QVariant();
    }
    return canCast( t );
}

/*!
    Compares this QVariant with \a v and returns true if they are
    equal; otherwise returns false.
*/

bool QVariant::operator==(const QVariant &v) const
{
    if (!v.canCast(type()))
	return false;
    switch(d->type) {
    case Cursor:
#ifndef QT_NO_CURSOR
	return v.toCursor().shape() == toCursor().shape();
#endif
    case Bitmap:
	return v.toBitmap().serialNumber() == toBitmap().serialNumber();
    case PointArray:
	return v.toPointArray() == toPointArray();
    case Region:
	return v.toRegion() == toRegion();
#ifndef QT_NO_TEMPLATE_VARIANT
    case List:
	return v.toList() == toList();
    case Map: {
	if (v.toMap().count() != toMap().count())
	    return false;
	QMap<QString, QVariant>::ConstIterator it = v.toMap().constBegin();
	QMap<QString, QVariant>::ConstIterator it2 = toMap().constBegin();
	while (it != v.toMap().constEnd()) {
	    if (*it != *it2)
		return false;
	    ++it;
	    ++it2;
	}
	return true;
    }
#endif
    case String:
	return v.toString() == toString();
#ifndef QT_NO_STRINGLIST
    case StringList:
	return v.toStringList() == toStringList();
#endif
    case Font:
	return v.toFont() == toFont();
    case Pixmap:
	return v.toPixmap().serialNumber() == toPixmap().serialNumber();
    case Image:
	return v.toImage() == toImage();
    case Brush:
	return v.toBrush() == toBrush();
    case Point:
	return v.toPoint() == toPoint();
    case Rect:
	return v.toRect() == toRect();
    case Size:
	return v.toSize() == toSize();
    case Color:
	return v.toColor() == toColor();
#ifndef QT_NO_PALETTE
    case Palette:
	return v.toPalette() == toPalette();
#ifndef QT_NO_COMPAT
    case ColorGroup:
	return v.toColorGroup() == toColorGroup();
#endif
#endif
#ifndef QT_NO_ICONSET
    case IconSet:
	return v.toIconSet().pixmap().serialNumber() == toIconSet().pixmap().serialNumber();
#endif
    case Int:
	return v.toInt() == toInt();
    case UInt:
	return v.toUInt() == toUInt();
    case LongLong:
	return v.toLongLong() == toLongLong();
    case ULongLong:
	return v.toULongLong() == toULongLong();
    case Bool:
	return v.toBool() == toBool();
    case Double:
	return v.toDouble() == toDouble();
    case SizePolicy:
	return v.toSizePolicy() == toSizePolicy();
    case Date:
	return v.toDate() == toDate();
    case Time:
	return v.toTime() == toTime();
    case DateTime:
	return v.toDateTime() == toDateTime();
    case ByteArray:
	return v.toByteArray() == toByteArray();
    case BitArray:
	return v.toBitArray() == toBitArray();
#ifndef QT_NO_ACCEL
    case KeySequence:
	return v.toKeySequence() == toKeySequence();
#endif
    case Pen:
	return v.toPen() == toPen();
    case Invalid:
	break;
    }
    return false;
}

/*!
    \fn bool QVariant::operator!=( const QVariant &v ) const
    Compares this QVariant with \a v and returns true if they are not
    equal; otherwise returns false.
*/

/*! \internal

  Reads or sets the variant type and ptr
 */
void *QVariant::rawAccess(void *ptr, Type typ, bool deepCopy)
{
    if (ptr) {
	clear();
	d->type = typ;
	d->value.ptr = ptr;
	d->is_null = false;
	if (deepCopy) {
	    Private *p = constructPrivate(d->type, data());
	    p->is_null = d->is_null;
	    cleanUp(d);
	    d = p;
	}
    }
    if (!deepCopy)
	return d->value.ptr;
    Private *p = constructPrivate(d->type, data());
    p->is_null = d->is_null;
    void *ret = (void*)p->value.ptr;
    p->type = Invalid;
    delete p;
    return ret;
}

/*! \internal
 */
void* QVariant::data()
{
    switch(d->type) {
    case Int:
    case UInt:
    case LongLong:
    case ULongLong:
    case Double:
    case Bool:
	return &d->value;
    default:
	return d->value.ptr;
    }
}

/*!
  Returns true if this is a NULL variant, false otherwise.
*/
bool QVariant::isNull() const
{
    switch( d->type ) {
    case Bitmap:
	return static_cast<QBitmap *>(d->value.ptr)->isNull();
    case Region:
	return static_cast<QRegion *>(d->value.ptr)->isEmpty();
    case PointArray:
	return static_cast<QPointArray *>(d->value.ptr)->isEmpty();
    case String:
	return static_cast<QString *>(d->value.ptr)->isNull();
    case Pixmap:
	return static_cast<QPixmap *>(d->value.ptr)->isNull();
    case Image:
	return static_cast<QImage *>(d->value.ptr)->isNull();
    case Point:
	return static_cast<QPoint *>(d->value.ptr)->isNull();
    case Rect:
	return static_cast<QRect *>(d->value.ptr)->isNull();
    case Size:
	return static_cast<QSize *>(d->value.ptr)->isNull();
#ifndef QT_NO_ICONSET
    case IconSet:
	return static_cast<QIconSet *>(d->value.ptr)->isNull();
#endif
    case Date:
	return static_cast<QDate *>(d->value.ptr)->isNull();
    case Time:
	return static_cast<QTime *>(d->value.ptr)->isNull();
    case DateTime:
	return static_cast<QDateTime *>(d->value.ptr)->isNull();
    case ByteArray:
	return static_cast<QByteArray *>(d->value.ptr)->isNull();
    case BitArray:
	return static_cast<QBitArray *>(d->value.ptr)->isNull();
    case Cursor:
#ifndef QT_NO_STRINGLIST
    case StringList:
#endif //QT_NO_STRINGLIST
    case Font:
    case Brush:
    case Color:
#ifndef QT_NO_PALETTE
    case Palette:
#ifndef QT_NO_COMPAT
    case ColorGroup:
#endif
#endif
#ifndef QT_NO_TEMPLATE_VARIANT
    case Map:
    case List:
#endif
    case SizePolicy:
#ifndef QT_NO_ACCEL
    case KeySequence:
#endif
    case Pen:
    case Invalid:
    case Int:
    case UInt:
    case LongLong:
    case ULongLong:
    case Bool:
    case Double:
	break;
    }
    return d->is_null;
}


QVariant::Private *QVariant::constructPrivate(Type type)
{
    Private *x = startConstruction();
    x->type = type;
    x->is_null = true;
    switch (type) {
    case Invalid:
	break;
    case Bitmap:
	x->value.ptr = new QBitmap;
	break;
    case Region:
	x->value.ptr = new QRegion;
	break;
    case PointArray:
	x->value.ptr = new QPointArray;
	break;
    case String:
	x->value.ptr = new QString;
	break;
#ifndef QT_NO_STRINGLIST
    case StringList:
	x->value.ptr = new QStringList;
	break;
#endif //QT_NO_STRINGLIST
    case Font:
	x->value.ptr = new QFont;
	break;
    case Pixmap:
	x->value.ptr = new QPixmap;
	break;
    case Image:
	// QImage is explicit shared
	x->value.ptr = new QImage;
	break;
    case Brush:
	x->value.ptr = new QBrush;
	// ## Force a detach
	// ((QBrush*)value.ptr)->setColor( ((QBrush*)value.ptr)->color() );
	break;
    case Point:
	x->value.ptr = new QPoint;
	break;
    case Rect:
	x->value.ptr = new QRect;
	break;
    case Size:
	x->value.ptr = new QSize;
	break;
    case Color:
	x->value.ptr = new QColor;
	break;
#ifndef QT_NO_PALETTE
    case Palette:
	x->value.ptr = new QPalette;
	break;
#ifndef QT_NO_COMPAT
    case ColorGroup:
	x->value.ptr = new QColorGroup;
	break;
#endif
#endif
#ifndef QT_NO_ICONSET
    case IconSet:
	x->value.ptr = new QIconSet;
	break;
#endif
#ifndef QT_NO_TEMPLATE_VARIANT
    case Map:
	x->value.ptr = new QMap<QString,QVariant>;
	break;
    case List:
	x->value.ptr = new QList<QVariant>;
	break;
#endif
    case Date:
	x->value.ptr = new QDate;
	break;
    case Time:
	x->value.ptr = new QTime;
	break;
    case DateTime:
	x->value.ptr = new QDateTime;
	break;
    case ByteArray:
	x->value.ptr = new QByteArray;
	break;
    case BitArray:
	x->value.ptr = new QBitArray;
	break;
#ifndef QT_NO_ACCEL
    case KeySequence:
	x->value.ptr = new QKeySequence;
	break;
#endif
    case Pen:
	x->value.ptr = new QPen;
	break;
    case Int:
	x->value.i = 0;
	break;
    case UInt:
	x->value.u = 0;
	break;
    case Bool:
	x->value.b = 0;
	break;
    case Double:
	x->value.d = 0;
	break;
    case SizePolicy:
	x->value.ptr = new QSizePolicy;
	break;
    case Cursor:
	x->value.ptr = new QCursor;
	break;
    case LongLong:
	x->value.ll = Q_LLONG(0);
	break;
    case ULongLong:
	x->value.ull = Q_ULLONG(0);
	break;	
    default:
	Q_ASSERT( 0 );
    }
    return x;
}

QVariant::Private *QVariant::constructPrivate(Type type, void *v)
{
    Private *x = startConstruction();
    x->type = type;
    x->is_null = true;
    switch( type ) {
    case Invalid:
	break;
    case Bitmap:
	x->value.ptr = new QBitmap(*static_cast<QBitmap *>(v));
	break;
    case Region:
	x->value.ptr = new QRegion(*static_cast<QRegion *>(v));
	break;
    case PointArray:
	x->value.ptr = new QPointArray(*static_cast<QPointArray *>(v));
	break;
    case String:
	x->value.ptr = new QString(*static_cast<QString *>(v));
	break;
#ifndef QT_NO_STRINGLIST
    case StringList:
	x->value.ptr = new QStringList(*static_cast<QStringList *>(v));
	break;
#endif //QT_NO_STRINGLIST
    case Font:
	x->value.ptr = new QFont(*static_cast<QFont *>(v));
	break;
    case Pixmap:
	x->value.ptr = new QPixmap(*static_cast<QPixmap *>(v));
	break;
    case Image:
	x->value.ptr = new QImage(*static_cast<QImage *>(v));
	break;
    case Brush:
	x->value.ptr = new QBrush(*static_cast<QBrush *>(v));
	break;
    case Point:
	x->value.ptr = new QPoint(*static_cast<QPoint *>(v));
	break;
    case Rect:
	x->value.ptr = new QRect(*static_cast<QRect *>(v));
	break;
    case Size:
	x->value.ptr = new QSize(*static_cast<QSize *>(v));
	break;
    case Color:
	x->value.ptr = new QColor(*static_cast<QColor *>(v));
	break;
#ifndef QT_NO_PALETTE
    case Palette:
	x->value.ptr = new QPalette(*static_cast<QPalette *>(v));
	break;
#ifndef QT_NO_COMPAT
    case ColorGroup:
	x->value.ptr = new QColorGroup(*static_cast<QColorGroup *>(v));
	break;
#endif
#endif
#ifndef QT_NO_ICONSET
    case IconSet:
	x->value.ptr = new QIconSet(*static_cast<QIconSet *>(v));
	break;
#endif
#ifndef QT_NO_TEMPLATE_VARIANT
    case Map:
	x->value.ptr = new QMap<QString,QVariant>(*static_cast<QMap<QString, QVariant> *>(v));
	break;
    case List:
	x->value.ptr = new QList<QVariant>(*static_cast<QList<QVariant> *>(v));
	break;
#endif
    case Date:
	x->value.ptr = new QDate(*static_cast<QDate *>(v));
	break;
    case Time:
	x->value.ptr = new QTime(*static_cast<QTime *>(v));
	break;
    case DateTime:
	x->value.ptr = new QDateTime(*static_cast<QDateTime *>(v));
	break;
    case ByteArray:
	x->value.ptr = new QByteArray(*static_cast<QByteArray *>(v));
	break;
    case BitArray:
	x->value.ptr = new QBitArray(*static_cast<QBitArray *>(v));
	break;
#ifndef QT_NO_ACCEL
    case KeySequence:
	x->value.ptr = new QKeySequence(*static_cast<QKeySequence *>(v));
	break;
#endif
    case Pen:
	x->value.ptr = new QPen(*static_cast<QPen *>(v));
	break;
    case Int:
	x->value.i = *static_cast<int *>(v);
	break;
    case UInt:
	x->value.u = *static_cast<uint *>(v);
	break;
    case Bool:
	x->value.b = *static_cast<bool *>(v);
	break;
    case Double:
	x->value.d = *static_cast<double *>(v);
	break;
    case SizePolicy:
	x->value.ptr = new QSizePolicy(*static_cast<QSizePolicy *>(v));
	break;
    case Cursor:
	x->value.ptr = new QCursor(*static_cast<QCursor *>(v));
	break;
    default:
	Q_ASSERT( 0 );
    }
    return x;
}
#endif //QT_NO_VARIANT

/****************************************************************************
**
** Definition of QVariant class.
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

#ifndef QVARIANT_H
#define QVARIANT_H

#ifndef QT_NO_VARIANT

#include "qcorevariant.h"
#include "qpixmap.h"
#include "qimage.h"
#include "qfont.h"
#include "qcolor.h"
#include "qpalette.h"
#include "qbrush.h"
#include "qpen.h"
#include "qrect.h"
#include "qsize.h"
#include "qpoint.h"
#include "qsizepolicy.h"
#include "qbitmap.h"
#include "qpointarray.h"
#include "qiconset.h"
#include "qkeysequence.h"
#include "qcursor.h"
#include "qregion.h"

class QFont;
class QPixmap;
class QBrush;
class QColor;
class QPalette;
#ifdef QT_COMPAT
class QColorGroup;
#endif
class QIconSet;
class QDataStream;
class QPointArray;
class QRegion;
class QBitmap;
class QCursor;
class QStringList;
class QSizePolicy;
class QKeySequence;
class QPen;
class QVariant;
class QApplicationPrivate;
class QPoint;
class QRect;
class QSize;

class Q_GUI_EXPORT QVariant : public QCoreVariant
{
 public:
    inline QVariant();

    inline QVariant(const QFont &font);
    QVariant(const QPixmap &pixmap);
    inline QVariant(const QImage &image);
    inline QVariant(const QBrush &brush);
    inline QVariant(const QColor &color);
#ifndef QT_NO_PALETTE
    inline QVariant(const QPalette &palette);
#ifdef QT_COMPAT
    inline QVariant(const QColorGroup &cg);
#endif // QT_COMPAT
#endif // QT_NO_PALETTE
#ifndef QT_NO_ICONSET
    inline QVariant(const QIconSet &iconset);
#endif
    inline QVariant(const QPointArray &pointarray);
    inline QVariant(const QRegion &region);
    inline QVariant(const QBitmap &bitmap);
#ifndef QT_NO_CURSOR
    inline QVariant(const QCursor &cursor);
#endif
#ifndef QT_NO_ACCEL
    inline QVariant(const QKeySequence &keysequence);
#endif
    inline QVariant(const QPen &pen);
    inline QVariant(const QSizePolicy &sp);

    // Copied from qcorevariant.h
    inline QVariant(Type type, void *v = 0);
    inline QVariant(const QCoreVariant &other);
    inline QVariant(const QVariant &other);

    inline QVariant(int i);
    inline QVariant(uint ui);
    inline QVariant(Q_LLONG ll);
    inline QVariant(Q_ULLONG ull);
    inline QVariant(double d);
    inline QVariant(bool b);
#ifdef QT_COMPAT
    inline QVariant(bool b, int);
#endif

    inline QVariant(const char *str);
    inline QVariant(const QByteArray &bytearray);
    inline QVariant(const QBitArray &bitarray);
    inline QVariant(const QString &string);
#ifndef QT_NO_STRINGLIST
    inline QVariant(const QStringList &stringlist);
#endif

    inline QVariant(const QDate &date);
    inline QVariant(const QTime &time);
    inline QVariant(const QDateTime &datetime);
#ifndef QT_NO_TEMPLATE_VARIANT
    inline QVariant(const QList<QVariant> &list);
    inline QVariant(const QList<QCoreVariant> &list);
    inline QVariant(const QMap<QString, QVariant> &map);
    inline QVariant(const QMap<QString,QCoreVariant> &map);
#endif

    inline QVariant(const QSize &size);
    inline QVariant(const QRect &rect);
    inline QVariant(const QPoint &pt);

    QFont toFont() const;
    QPixmap toPixmap() const;
    const QImage toImage() const;
    QBrush toBrush() const;
    QColor toColor() const;
    QPalette toPalette() const;
#ifdef QT_COMPAT
    QT_COMPAT QColorGroup toColorGroup() const;
#endif
    QIconSet toIconSet() const;
    const QPointArray toPointArray() const;
    QBitmap toBitmap() const;
    QRegion toRegion() const;
#ifndef QT_NO_CURSOR
    QCursor toCursor() const;
#endif
#ifndef QT_NO_ACCEL
    QKeySequence toKeySequence() const;
#endif
    QPen toPen() const;
    QSizePolicy toSizePolicy() const;

    QPoint toPoint() const;
    QRect toRect() const;
    QSize toSize() const;


#ifdef QT_COMPAT
    QT_COMPAT QPoint &asPoint();
    QT_COMPAT QRect &asRect();
    QT_COMPAT QSize &asSize();
    QT_COMPAT QFont &asFont();
    QT_COMPAT QPixmap &asPixmap();
    QT_COMPAT QImage &asImage();
    QT_COMPAT QBrush &asBrush();
    QT_COMPAT QColor &asColor();
#ifndef QT_NO_PALETTE
    QT_COMPAT QPalette &asPalette();
    QT_COMPAT QColorGroup &asColorGroup();
#endif // QT_NO_PALETTE
    QT_COMPAT QIconSet &asIconSet();
    QT_COMPAT QPointArray &asPointArray();
    QT_COMPAT QBitmap &asBitmap();
    QT_COMPAT QRegion &asRegion();
#ifndef QT_NO_CURSOR
    QT_COMPAT QCursor &asCursor();
#endif // QT_NO_CURSOR
#ifndef QT_NO_ACCEL
    QT_COMPAT QKeySequence &asKeySequence();
#endif // QT_NO_ACCEL
    QT_COMPAT QPen &asPen();
    QT_COMPAT QSizePolicy &asSizePolicy();
#endif // QT_COMPAT

private:
    friend class QApplicationPrivate;
};

// Copied from qcorevariant.h
inline QVariant::QVariant(Type type, void *v) : QCoreVariant(type, v) { }
inline QVariant::QVariant(const QVariant &other) : QCoreVariant(other) { }
inline QVariant::QVariant(const QCoreVariant &other) : QCoreVariant(other) { }

inline QVariant::QVariant(int i) : QCoreVariant(i) {};
inline QVariant::QVariant(uint ui) : QCoreVariant(ui) {};
inline QVariant::QVariant(Q_LLONG ll) : QCoreVariant(ll) {};
inline QVariant::QVariant(Q_ULLONG ull) : QCoreVariant(ull) {};
inline QVariant::QVariant(bool b) : QCoreVariant(b) {};
inline QVariant::QVariant(double d) : QCoreVariant(d) {};
#ifdef QT_COMPAT
inline QVariant::QVariant(bool b, int) : QCoreVariant(b) {};
#endif

inline QVariant::QVariant(const char *str) : QCoreVariant(str) {};
inline QVariant::QVariant(const QByteArray &bytearray) : QCoreVariant(bytearray) {};
inline QVariant::QVariant(const QBitArray &bitarray) : QCoreVariant(bitarray) {};
inline QVariant::QVariant(const QString &string) : QCoreVariant(string) {};
#ifndef QT_NO_STRINGLIST
inline QVariant::QVariant(const QStringList &stringlist) : QCoreVariant(stringlist) {};
#endif

inline QVariant::QVariant(const QDate &date) : QCoreVariant(date) {};
inline QVariant::QVariant(const QTime &time) : QCoreVariant(time) {};
inline QVariant::QVariant(const QDateTime &datetime) : QCoreVariant(datetime) {};
#ifndef QT_NO_TEMPLATE_VARIANT
inline QVariant::QVariant(const QList<QVariant> &list)
    : QCoreVariant(*reinterpret_cast<const QList<QCoreVariant>*>(&list)) {};
inline QVariant::QVariant(const QList<QCoreVariant> &list) : QCoreVariant(list) {};
inline QVariant::QVariant(const QMap<QString, QVariant> &map)
    : QCoreVariant(*reinterpret_cast<const QMap<QString, QCoreVariant>*>(&map)) {};
inline QVariant::QVariant(const QMap<QString, QCoreVariant> &map) : QCoreVariant(map) {};
#endif
inline QVariant::QVariant(const QPoint &pt)
{ d = create(Point, &pt); }
inline QVariant::QVariant(const QRect &r)
{ d = create(Rect, &r); }
inline QVariant::QVariant(const QSize &s)
{ d = create(Size, &s); }

inline QVariant::QVariant()
    : QCoreVariant()
{ }
inline QVariant::QVariant(const QFont &val)
{ d = create(Font, &val); }
inline QVariant::QVariant(const QImage &val)
{ d = create(Image, &val); }
inline QVariant::QVariant(const QBrush &val)
{ d = create(Brush, &val); }
inline QVariant::QVariant(const QColor &val)
{ d = create(Color, &val); }
#ifndef QT_NO_PALETTE
inline QVariant::QVariant(const QPalette &val)
{ d = create(Palette, &val); }
#ifdef QT_COMPAT
inline QVariant::QVariant(const QColorGroup &val)
{ d = create(ColorGroup, &val); }
#endif
#endif //QT_NO_PALETTE
#ifndef QT_NO_ICONSET
inline QVariant::QVariant(const QIconSet &val)
{ d = create(IconSet, &val); }
#endif //QT_NO_ICONSET
inline QVariant::QVariant(const QPointArray &val)
{ d = create(PointArray, &val); }
inline QVariant::QVariant(const QRegion &val)
{ d = create(Region, &val); }
inline QVariant::QVariant(const QBitmap& val)
{ d = create(Bitmap, &val); }
#ifndef QT_NO_CURSOR
inline QVariant::QVariant(const QCursor &val)
{ d = create(Cursor, &val); }
#endif
#ifndef QT_NO_ACCEL
inline QVariant::QVariant(const QKeySequence &val)
{ d = create(KeySequence, &val); }
#endif
inline QVariant::QVariant(const QPen &val)
{ d = create(Pen, &val); }
inline QVariant::QVariant(const QSizePolicy &val)
{ d = create(SizePolicy, &val); }

#ifdef QT_COMPAT
inline QFont& QVariant::asFont()
{ return *static_cast<QFont *>(castOrDetach(Font)); }
inline QImage& QVariant::asImage()
{ return *static_cast<QImage *>(castOrDetach(Image)); }
inline QBrush& QVariant::asBrush()
{ return *static_cast<QBrush *>(castOrDetach(Brush)); }
inline QColor& QVariant::asColor()
{ return *static_cast<QColor *>(castOrDetach(Color)); }
#ifndef QT_NO_PALETTE
inline QPalette& QVariant::asPalette()
{ return *static_cast<QPalette *>(castOrDetach(Palette)); }
inline QColorGroup& QVariant::asColorGroup()
{ return *static_cast<QColorGroup *>(castOrDetach(ColorGroup)); }
#endif // QT_NO_PALETTE
#ifndef QT_NO_ICONSET
inline QIconSet& QVariant::asIconSet()
{ return *static_cast<QIconSet *>(castOrDetach(IconSet)); }
#endif
inline QPointArray& QVariant::asPointArray()
{ return *static_cast<QPointArray *>(castOrDetach(PointArray)); }
inline QBitmap& QVariant::asBitmap()
{ return *static_cast<QBitmap *>(castOrDetach(Bitmap)); }
inline QRegion& QVariant::asRegion()
{ return *static_cast<QRegion *>(castOrDetach(Region)); }
#ifndef QT_NO_CURSOR
inline QCursor& QVariant::asCursor()
{ return *static_cast<QCursor *>(castOrDetach(Cursor)); }
#endif
#ifndef QT_NO_ACCEL
inline QKeySequence& QVariant::asKeySequence()
{ return *static_cast<QKeySequence *>(castOrDetach(KeySequence)); }
#endif
inline QPen& QVariant::asPen()
{ return *static_cast<QPen *>(castOrDetach(Pen)); }
inline QSizePolicy& QVariant::asSizePolicy()
{ return *static_cast<QSizePolicy *>(castOrDetach(SizePolicy)); }


inline QPoint& QVariant::asPoint()
{ return *static_cast<QPoint *>(castOrDetach(Point)); }

inline QRect& QVariant::asRect()
{ return *static_cast<QRect *>(castOrDetach(Rect)); }

inline QSize &QVariant::asSize()
{ return *static_cast<QSize *>(castOrDetach(Size)); }
#endif //QT_COMPAT


inline QPoint QVariant::toPoint() const
{
    if (d->type != Point)
	return QPoint();

    return *static_cast<QPoint *>(d->value.ptr);
}

inline QRect QVariant::toRect() const
{
    if (d->type != Rect)
	return QRect();

    return *static_cast<QRect *>(d->value.ptr);
}

inline QSize QVariant::toSize() const
{
    if (d->type != Size)
	return QSize();

    return *static_cast<QSize *>(d->value.ptr);
}


inline const QImage QVariant::toImage() const
{
    if (d->type != Image)
	return QImage();

    return *static_cast<QImage *>(d->value.ptr);
}

inline QBrush QVariant::toBrush() const
{
    if (d->type != Brush)
	return QBrush();

    return *static_cast<QBrush *>(d->value.ptr);
}


#ifndef QT_NO_PALETTE
inline QPalette QVariant::toPalette() const
{
    if (d->type != Palette)
	return QPalette();

    return *static_cast<QPalette *>(d->value.ptr);
}

#ifdef QT_COMPAT
inline QColorGroup QVariant::toColorGroup() const
{
    if (d->type != ColorGroup)
	return QColorGroup();
    return *static_cast<QColorGroup *>(d->value.ptr);
}
#endif
#endif //QT_NO_PALETTE

inline QPen QVariant::toPen() const
{
    if (d->type != Pen)
	return QPen();

    return *static_cast<QPen*>(d->value.ptr);
}

inline QSizePolicy QVariant::toSizePolicy() const
{
    if (d->type == SizePolicy)
	return *static_cast<QSizePolicy *>(d->value.ptr);

    return QSizePolicy();
}

#ifndef QT_NO_CURSOR
inline QCursor QVariant::toCursor() const
{
    if (d->type != Cursor)
	return QCursor();

    return *static_cast<QCursor *>(d->value.ptr);
}
#endif

inline QRegion QVariant::toRegion() const
{
    if (d->type != Region)
	return QRegion();

    return *static_cast<QRegion *>(d->value.ptr);
}

inline QBitmap QVariant::toBitmap() const
{
    if (d->type != Bitmap)
	return QBitmap();

    return *static_cast<QBitmap *>(d->value.ptr);

}

inline const QPointArray QVariant::toPointArray() const
{
    if (d->type != PointArray)
	return QPointArray();

    return *static_cast<QPointArray *>(d->value.ptr);
}

#ifndef QT_NO_ICONSET
inline QIconSet QVariant::toIconSet() const
{
    if (d->type != IconSet)
	return QIconSet();

    return *static_cast<QIconSet *>(d->value.ptr);
}
#endif //QT_NO_ICONSET


#define Q_VARIANT_TO(f) \
inline Q##f QVariant::to##f() const { \
    if ( d->type == f ) \
        return *static_cast<Q##f *>(d->value.ptr); \
    Q##f ret; \
    handler->cast(d, f, &ret, 0); \
    return ret; \
}

Q_VARIANT_TO(Font);
Q_VARIANT_TO(Color);
#ifndef QT_NO_ACCEL
Q_VARIANT_TO(KeySequence);
#endif

template<typename T> T qt_cast(const QVariant &v, const T* = 0);
template<> inline int qt_cast<int>(const QVariant &v, const int*) { return v.toInt(); }
template<> inline uint qt_cast<uint>(const QVariant &v, const uint*) { return v.toUInt(); }
template<> inline Q_LLONG qt_cast<Q_LLONG>(const QVariant &v, const Q_LLONG*) { return v.toLongLong(); }
template<> inline Q_ULLONG qt_cast<Q_ULLONG>(const QVariant &v, const Q_ULLONG*) { return v.toULongLong(); }
template<> inline bool qt_cast<bool>(const QVariant &v, const bool*) { return v.toBool(); }
template<> inline double qt_cast<double>(const QVariant &v, const double*) { return v.toDouble(); }
template<> inline QByteArray qt_cast<QByteArray>(const QVariant &v, const QByteArray*) { return v.toByteArray(); }

template<> QString qt_cast<QString>(const QVariant &v, const QString*);
template<> QBitArray qt_cast<QBitArray>(const QVariant &v, const QBitArray*);
#ifndef QT_NO_STRINGLIST
template<> QStringList qt_cast<QStringList>(const QVariant &v, const QStringList*);
#endif
template<> QDate qt_cast<QDate>(const QVariant &v, const QDate*);
template<> QTime qt_cast<QTime>(const QVariant &v, const QTime*);
template<> QDateTime qt_cast<QDateTime>(const QVariant &v, const QDateTime*);
#ifndef QT_NO_TEMPLATE_VARIANT
template<> QList<QVariant> qt_cast<QList<QVariant> >(const QVariant &v, const QList<QVariant>*);
template<> QMap<QString,QVariant> qt_cast<QMap<QString,QVariant> >(const QVariant &v, const QMap<QString,QVariant>*);
#endif

template<> inline QFont qt_cast<QFont>(const QVariant &v, const QFont*) { return v.toFont(); }
template<> inline QPixmap qt_cast<QPixmap>(const QVariant &v, const QPixmap*) { return v.toPixmap(); }
template<> inline QImage qt_cast<QImage>(const QVariant &v, const QImage*) { return v.toImage(); }
template<> inline QBrush qt_cast<QBrush>(const QVariant &v, const QBrush*) { return v.toBrush(); }
template<> inline QColor qt_cast<QColor>(const QVariant &v, const QColor*) { return v.toColor(); }
template<> inline QPalette qt_cast<QPalette>(const QVariant &v, const QPalette*) { return v.toPalette(); }
template<> inline QIconSet qt_cast<QIconSet>(const QVariant &v, const QIconSet*) { return v.toIconSet(); }
template<> inline QPointArray qt_cast<QPointArray>(const QVariant &v, const QPointArray*)
{ return v.toPointArray(); }
template<> inline QBitmap qt_cast<QBitmap>(const QVariant &v, const QBitmap*) { return v.toBitmap(); }
template<> inline QRegion qt_cast<QRegion>(const QVariant &v, const QRegion*) { return v.toRegion(); }
#ifndef QT_NO_CURSOR
template<> inline QCursor qt_cast<QCursor>(const QVariant &v, const QCursor*) { return v.toCursor(); }
#endif
#ifndef QT_NO_ACCEL
template<> inline QKeySequence qt_cast<QKeySequence>(const QVariant &v, const QKeySequence*)
{ return v.toKeySequence(); }
#endif
template<> inline QPen qt_cast<QPen>(const QVariant &v, const QPen*) { return v.toPen(); }
template<> inline QSizePolicy qt_cast<QSizePolicy>(const QVariant &v, const QSizePolicy*) { return v.toSizePolicy(); }
template<> inline QPoint qt_cast<QPoint>(const QVariant &v, const QPoint*) { return v.toPoint(); }
template<> inline QRect qt_cast<QRect>(const QVariant &v, const QRect*) { return v.toRect(); }
template<> inline QSize qt_cast<QSize>(const QVariant &v, const QSize*) { return v.toSize(); }

Q_DECLARE_TYPEINFO(QVariant, Q_MOVABLE_TYPE);
Q_DECLARE_SHARED(QVariant);

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug, const QVariant &);
#endif

#endif // QT_NO_VARIANT
#endif // QVARIANT_H

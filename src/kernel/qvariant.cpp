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

#include <qvariant.h>

#include "qbitmap.h"
#include "qbrush.h"
#include "qcolor.h"
#include "qcursor.h"
#include "qdatastream.h"
#include "qdebug.h"
#include "qfont.h"
#include "qiconset.h"
#include "qimage.h"
#include "qkeysequence.h"
#include "qpalette.h"
#include "qpen.h"
#include "qpixmap.h"
#include "qpoint.h"
#include "qpointarray.h"
#include "qrect.h"
#include "qregion.h"
#include "qsize.h"
#include "qsizepolicy.h"

extern const QVariant::Handler qt_gui_variant_handler;
Q_CORE_EXPORT const QVariant::Handler *qcoreVariantHandler();

template<typename T> inline T *v_cast(void *&p)
{
    if (QTypeInfo<T>::isLarge)
        return static_cast<T*>(p);
    else
        return reinterpret_cast<T*>(&p);
}

static void construct(QVariant::Private *x, const void *v)
{
    if (v) {
	switch( x->type ) {
	case QVariant::Bitmap:
	    x->value.ptr = new QBitmap(*static_cast<const QBitmap *>(v));
	    break;
	case QVariant::Region:
	    x->value.ptr = new QRegion(*static_cast<const QRegion *>(v));
	    break;
	case QVariant::PointArray:
	    x->value.ptr = new QPointArray(*static_cast<const QPointArray *>(v));
	    break;
	case QVariant::Font:
	    x->value.ptr = new QFont(*static_cast<const QFont *>(v));
	    break;
	case QVariant::Pixmap:
	    x->value.ptr = new QPixmap(*static_cast<const QPixmap *>(v));
	    break;
	case QVariant::Image:
	    x->value.ptr = new QImage(*static_cast<const QImage *>(v));
	    break;
	case QVariant::Brush:
	    x->value.ptr = new QBrush(*static_cast<const QBrush *>(v));
	    break;
	case QVariant::Point:
	    x->value.ptr = new QPoint(*static_cast<const QPoint *>(v));
	    break;
	case QVariant::Rect:
	    x->value.ptr = new QRect(*static_cast<const QRect *>(v));
	    break;
	case QVariant::Size:
	    x->value.ptr = new QSize(*static_cast<const QSize *>(v));
	    break;
	case QVariant::Color:
	    x->value.ptr = new QColor(*static_cast<const QColor *>(v));
	    break;
#ifndef QT_NO_PALETTE
	case QVariant::Palette:
	    x->value.ptr = new QPalette(*static_cast<const QPalette *>(v));
	    break;
#ifdef QT_COMPAT
	case QVariant::ColorGroup:
	    x->value.ptr = new QColorGroup(*static_cast<const QColorGroup *>(v));
	    break;
#endif
#endif
#ifndef QT_NO_ICONSET
	case QVariant::IconSet:
	    x->value.ptr = new QIconSet(*static_cast<const QIconSet *>(v));
	    break;
#endif
#ifndef QT_NO_ACCEL
	case QVariant::KeySequence:
	    x->value.ptr = new QKeySequence(*static_cast<const QKeySequence *>(v));
	    break;
#endif
	case QVariant::Pen:
	    x->value.ptr = new QPen(*static_cast<const QPen *>(v));
	    break;
	case QVariant::SizePolicy:
	    x->value.ptr = new QSizePolicy(*static_cast<const QSizePolicy *>(v));
	    break;
	case QVariant::Cursor:
	    x->value.ptr = new QCursor(*static_cast<const QCursor *>(v));
	    break;
	default:
	    qcoreVariantHandler()->construct(x, v);
	}
	x->is_null = false;
    } else {
	switch (x->type) {
	case QVariant::Bitmap:
	    x->value.ptr = new QBitmap;
	    break;
	case QVariant::Region:
	    x->value.ptr = new QRegion;
	    break;
	case QVariant::PointArray:
	    x->value.ptr = new QPointArray;
	    break;
	case QVariant::Font:
	    x->value.ptr = new QFont;
	    break;
	case QVariant::Pixmap:
	    x->value.ptr = new QPixmap;
	    break;
	case QVariant::Image:
	    // QImage is explicit shared
	    x->value.ptr = new QImage;
	    break;
	case QVariant::Brush:
	    x->value.ptr = new QBrush;
	    // ## Force a detach
	    // ((QBrush*)value.ptr)->setColor( ((QBrush*)value.ptr)->color() );
	    break;
	case QVariant::Point:
	    x->value.ptr = new QPoint;
	    break;
	case QVariant::Rect:
	    x->value.ptr = new QRect;
	    break;
	case QVariant::Size:
	    x->value.ptr = new QSize;
	    break;
	case QVariant::Color:
	    x->value.ptr = new QColor;
	    break;
#ifndef QT_NO_PALETTE
	case QVariant::Palette:
	    x->value.ptr = new QPalette;
	    break;
#ifdef QT_COMPAT
	case QVariant::ColorGroup:
	    x->value.ptr = new QColorGroup;
	    break;
#endif
#endif
#ifndef QT_NO_ICONSET
	case QVariant::IconSet:
	    x->value.ptr = new QIconSet;
	    break;
#endif
#ifndef QT_NO_ACCEL
	case QVariant::KeySequence:
	    x->value.ptr = new QKeySequence;
	    break;
#endif
	case QVariant::Pen:
	    x->value.ptr = new QPen;
	    break;
	case QVariant::SizePolicy:
	    x->value.ptr = new QSizePolicy;
	    break;
#ifndef QT_NO_CURSOR
	case QVariant::Cursor:
	    x->value.ptr = new QCursor;
	    break;
#endif
	default:
	    qcoreVariantHandler()->construct(x, v);
	}

    }
}



static void clear(QVariant::Private *p)
{
    switch (p->type) {
    case QVariant::Bitmap:
	delete static_cast<QBitmap *>(p->value.ptr);
	break;
    case QVariant::Cursor:
	delete static_cast<QCursor *>(p->value.ptr);
	break;
    case QVariant::Region:
	delete static_cast<QRegion *>(p->value.ptr);
	break;
    case QVariant::PointArray:
	delete static_cast<QPointArray *>(p->value.ptr);
	break;
    case QVariant::Font:
	delete static_cast<QFont *>(p->value.ptr);
	break;
    case QVariant::Pixmap:
	delete static_cast<QPixmap *>(p->value.ptr);
	break;
    case QVariant::Image:
	delete static_cast<QImage *>(p->value.ptr);
	break;
    case QVariant::Brush:
	delete static_cast<QBrush *>(p->value.ptr);
	break;
    case QVariant::Point:
	delete static_cast<QPoint *>(p->value.ptr);
	break;
    case QVariant::Rect:
	delete static_cast<QRect *>(p->value.ptr);
	break;
    case QVariant::Size:
	delete static_cast<QSize *>(p->value.ptr);
	break;
    case QVariant::Color:
	delete static_cast<QColor *>(p->value.ptr);
	break;
#ifndef QT_NO_PALETTE
    case QVariant::Palette:
	delete static_cast<QPalette *>(p->value.ptr);
	break;
#ifdef QT_COMPAT
    case QVariant::ColorGroup:
	delete static_cast<QColorGroup *>(p->value.ptr);
	break;
#endif
#endif
#ifndef QT_NO_ICONSET
    case QVariant::IconSet:
	delete static_cast<QIconSet *>(p->value.ptr);
	break;
#endif
    case QVariant::SizePolicy:
	delete static_cast<QSizePolicy *>(p->value.ptr);
	break;
#ifndef QT_NO_ACCEL
    case QVariant::KeySequence:
	delete static_cast<QKeySequence *>(p->value.ptr);
	break;
#endif
    case QVariant::Pen:
	delete static_cast<QPen *>(p->value.ptr);
	break;
    default:
	qcoreVariantHandler()->clear(p);
	break;
    }

    p->type = QVariant::Invalid;
    p->is_null = true;
    if (p->str_cache) {
        reinterpret_cast<QString *>(&p->str_cache)->~QString();
        p->str_cache = 0;
    }
}


static bool isNull(const QVariant::Private *d)
{
    switch( d->type ) {
    case QVariant::Bitmap:
	return static_cast<QBitmap *>(d->value.ptr)->isNull();
    case QVariant::Region:
	return static_cast<QRegion *>(d->value.ptr)->isEmpty();
    case QVariant::PointArray:
	return static_cast<QPointArray *>(d->value.ptr)->isEmpty();
    case QVariant::Pixmap:
	return static_cast<QPixmap *>(d->value.ptr)->isNull();
    case QVariant::Image:
	return static_cast<QImage *>(d->value.ptr)->isNull();
    case QVariant::Point:
	return static_cast<QPoint *>(d->value.ptr)->isNull();
    case QVariant::Rect:
	return static_cast<QRect *>(d->value.ptr)->isNull();
    case QVariant::Size:
	return static_cast<QSize *>(d->value.ptr)->isNull();
#ifndef QT_NO_ICONSET
    case QVariant::IconSet:
	return static_cast<QIconSet *>(d->value.ptr)->isNull();
#endif
    case QVariant::Cursor:
#ifndef QT_NO_STRINGLIST
    case QVariant::StringList:
#endif //QT_NO_STRINGLIST
    case QVariant::Font:
    case QVariant::Brush:
    case QVariant::Color:
#ifndef QT_NO_PALETTE
    case QVariant::Palette:
#ifdef QT_COMPAT
    case QVariant::ColorGroup:
#endif
#endif
    case QVariant::SizePolicy:
#ifndef QT_NO_ACCEL
    case QVariant::KeySequence:
#endif
    case QVariant::Pen:
	break;
    default:
	return qcoreVariantHandler()->isNull(d);
    }
    return d->is_null;
}

#ifndef QT_NO_DATASTREAM
static void load(QVariant::Private *d, QDataStream &s)
{
    switch (d->type) {
#ifndef QT_NO_CURSOR
    case QVariant::Cursor:
	s >> *static_cast<QCursor *>(d->value.ptr);
	break;
#endif
#ifndef QT_NO_IMAGEIO
    case QVariant::Bitmap: {
	s >> *static_cast<QBitmap *>(d->value.ptr);
	break;
#endif
    case QVariant::Region:
	s >> *static_cast<QRegion *>(d->value.ptr);
	break;
    case QVariant::PointArray:
	s >> *static_cast<QPointArray *>(d->value.ptr);
	break;
    case QVariant::Font:
	s >> *static_cast<QFont *>(d->value.ptr);
	break;
#ifndef QT_NO_IMAGEIO
    case QVariant::Pixmap:
	s >> *static_cast<QPixmap *>(d->value.ptr);
	break;
    case QVariant::Image:
	s >> *static_cast<QImage *>(d->value.ptr);
	break;
#endif
    case QVariant::Brush:
	s >> *static_cast<QBrush *>(d->value.ptr);
	break;
    case QVariant::Rect:
	s >> *static_cast<QRect *>(d->value.ptr);
	break;
    case QVariant::Point:
	s >> *static_cast<QPoint *>(d->value.ptr);
	break;
    case QVariant::Size:
	s >> *static_cast<QSize *>(d->value.ptr);
	break;
    case QVariant::Color:
	s >> *static_cast<QColor *>(d->value.ptr);
	break;
#ifndef QT_NO_PALETTE
    case QVariant::Palette:
	s >> *static_cast<QPalette *>(d->value.ptr);
	break;
#ifdef QT_COMPAT
    case QVariant::ColorGroup:
	s >> *static_cast<QColorGroup *>(d->value.ptr);
	break;
#endif
#endif
#ifndef QT_NO_ICONSET
    case QVariant::IconSet:
	QPixmap x;
	s >> x;
	*static_cast<QIconSet *>(d->value.ptr) = QIconSet(x);
	break;
    }
#endif
    case QVariant::SizePolicy:
	int h, v;
	Q_INT8 hfw;
	s >> h >> v >> hfw;
	*static_cast<QSizePolicy *>(d->value.ptr) =
	    QSizePolicy((QSizePolicy::SizeType)h, (QSizePolicy::SizeType)v, (bool)hfw);
	break;
#ifndef QT_NO_ACCEL
    case QVariant::KeySequence:
	s >> *static_cast<QKeySequence *>(d->value.ptr);
	break;
#endif // QT_NO_ACCEL
    case QVariant::Pen:
	s >> *static_cast<QPen *>(d->value.ptr);
	break;
    default:
	qcoreVariantHandler()->load(d, s);
	return;
    }
}


static void save(const QVariant::Private *d, QDataStream &s)
{
    switch (d->type) {
    case QVariant::Cursor:
	s << *static_cast<QCursor *>(d->value.ptr);
	break;
    case QVariant::Bitmap:
#ifndef QT_NO_IMAGEIO
	s << *static_cast<QBitmap *>(d->value.ptr);
#endif
	break;
    case QVariant::PointArray:
	s << *static_cast<QPointArray *>(d->value.ptr);
	break;
    case QVariant::Region:
	s << *static_cast<QRegion *>(d->value.ptr);
	break;
    case QVariant::Font:
	s << *static_cast<QFont *>(d->value.ptr);
	break;
    case QVariant::Pixmap:
#ifndef QT_NO_IMAGEIO
	s << *static_cast<QPixmap *>(d->value.ptr);
#endif
	break;
    case QVariant::Image:
#ifndef QT_NO_IMAGEIO
	s << *static_cast<QImage *>(d->value.ptr);
#endif
	break;
    case QVariant::Brush:
	s << *static_cast<QBrush *>(d->value.ptr);
	break;
    case QVariant::Point:
	s << *static_cast<QPoint *>(d->value.ptr);
	break;
    case QVariant::Rect:
	s << *static_cast<QRect *>(d->value.ptr);
	break;
    case QVariant::Size:
	s << *static_cast<QSize *>(d->value.ptr);
	break;
    case QVariant::Color:
	s << *static_cast<QColor *>(d->value.ptr);
	break;
#ifndef QT_NO_PALETTE
    case QVariant::Palette:
	s << *static_cast<QPalette *>(d->value.ptr);
	break;
#ifdef QT_COMPAT
    case QVariant::ColorGroup:
	s << *static_cast<QColorGroup *>(d->value.ptr);
	break;
#endif
#endif
#ifndef QT_NO_ICONSET
    case QVariant::IconSet:
	//### add stream operator to iconset
	s << static_cast<QIconSet *>(d->value.ptr)->pixmap();
	break;
#endif
    case QVariant::SizePolicy:
	{
	    QSizePolicy *p = static_cast<QSizePolicy *>(d->value.ptr);
	    s << (int) p->horData() << (int) p->verData()
	      << (Q_INT8) p->hasHeightForWidth();
	}
	break;
#ifndef QT_NO_ACCEL
    case QVariant::KeySequence:
	s << *static_cast<QKeySequence *>(d->value.ptr);
	break;
#endif
    case QVariant::Pen:
	s << *static_cast<QPen *>(d->value.ptr);
	break;
    default:
	qcoreVariantHandler()->save(d, s);
    }
}
#endif


static bool compare(const QVariant::Private *a, const QVariant::Private *b)
{
    Q_ASSERT(a->type == b->type);
    switch(a->type) {
    case QVariant::Cursor:
#ifndef QT_NO_CURSOR
	return static_cast<QCursor *>(a->value.ptr)->shape()
	    == static_cast<QCursor *>(b->value.ptr)->shape();
#endif
    case QVariant::Bitmap:
	return static_cast<QBitmap *>(a->value.ptr)->serialNumber()
	    == static_cast<QBitmap *>(b->value.ptr)->serialNumber();
    case QVariant::PointArray:
	return *static_cast<QPointArray *>(a->value.ptr)
	    == *static_cast<QPointArray *>(b->value.ptr);
    case QVariant::Region:
	return *static_cast<QRegion *>(a->value.ptr)
	    == *static_cast<QRegion *>(b->value.ptr);
    case QVariant::Font:
	return *static_cast<QFont *>(a->value.ptr)
	    == *static_cast<QFont *>(b->value.ptr);
    case QVariant::Pixmap:
	return static_cast<QPixmap *>(a->value.ptr)->serialNumber()
	    == static_cast<QPixmap *>(b->value.ptr)->serialNumber();
    case QVariant::Image:
	return *static_cast<QImage *>(a->value.ptr)
	    == *static_cast<QImage *>(b->value.ptr);
    case QVariant::Brush:
	return *static_cast<QBrush *>(a->value.ptr)
	    == *static_cast<QBrush *>(b->value.ptr);
    case QVariant::Point:
	return *static_cast<QPoint *>(a->value.ptr)
	    == *static_cast<QPoint *>(b->value.ptr);
    case QVariant::Rect:
	return *static_cast<QRect *>(a->value.ptr)
	    == *static_cast<QRect *>(b->value.ptr);
    case QVariant::Size:
	return *static_cast<QSize *>(a->value.ptr)
	    == *static_cast<QSize *>(b->value.ptr);
    case QVariant::Color:
	return *static_cast<QColor *>(a->value.ptr)
	    == *static_cast<QColor *>(b->value.ptr);
#ifndef QT_NO_PALETTE
    case QVariant::Palette:
	return *static_cast<QPalette *>(a->value.ptr)
	    == *static_cast<QPalette *>(b->value.ptr);
#ifdef QT_COMPAT
    case QVariant::ColorGroup:
	return *static_cast<QColorGroup *>(a->value.ptr)
	    == *static_cast<QColorGroup *>(b->value.ptr);
#endif
#endif
#ifndef QT_NO_ICONSET
    case QVariant::IconSet:
	return static_cast<QIconSet *>(a->value.ptr)->pixmap().serialNumber()
	    == static_cast<QIconSet *>(b->value.ptr)->pixmap().serialNumber();
#endif
    case QVariant::SizePolicy:
	return *static_cast<QSizePolicy *>(a->value.ptr)
	    == *static_cast<QSizePolicy *>(b->value.ptr);
#ifndef QT_NO_ACCEL
    case QVariant::KeySequence:
	return *static_cast<QKeySequence *>(a->value.ptr)
	    == *static_cast<QKeySequence *>(b->value.ptr);
#endif
    case QVariant::Pen:
	return *static_cast<QPen *>(a->value.ptr)
	    == *static_cast<QPen *>(b->value.ptr);
    default:
	break;
    }
    return qcoreVariantHandler()->compare(a, b);
}



static void cast(QVariant::Private *d, QVariant::Type t, void *result, bool *ok)
{
    bool converted = false;
    switch (t) {
    case QVariant::String: {
	QString *str = static_cast<QString *>(result);
	switch (d->type) {
#ifndef QT_NO_ACCEL
	case QVariant::KeySequence:
	    *str = QString(*static_cast<QKeySequence *>(d->value.ptr));
	    converted = true;
	    break;
#endif
	case QVariant::Font:
	    *str = static_cast<QFont *>(d->value.ptr)->toString();
	    converted = true;
	    break;
	case QVariant::Color:
	    *str = static_cast<QColor *>(d->value.ptr)->name();
	    converted = true;
	    break;
	default:
	    break;
	}
	break;
    }
#ifndef QT_NO_ACCEL
    case QVariant::Int:
	if (d->type == QVariant::KeySequence) {
	    *static_cast<int *>(result) = (int)(*(static_cast<QKeySequence*>(d->value.ptr)));
	    converted = true;
	}
	break;
#endif
    case QVariant::Font:
	if (d->type == QVariant::String) {
	    QFont *f = static_cast<QFont *>(result);
	    f->fromString(*v_cast<QString>(d->value.ptr));
	    converted = true;
	}
	break;
    case QVariant::Color:
	if (d->type == QVariant::String) {
	    static_cast<QColor *>(result)->setNamedColor(*v_cast<QString>(d->value.ptr));
	    converted = true;
	}
#ifndef QT_NO_ACCEL
    case QVariant::KeySequence: {
	QKeySequence *seq = static_cast<QKeySequence *>(result);
	switch (d->type) {
	case QVariant::String:
	    *seq = QKeySequence(*v_cast<QString>(d->value.ptr));
	    converted = true;
	    break;
	case QVariant::Int:
	    *seq = QKeySequence(d->value.i);
	    converted = true;
	    break;
	default:
	    break;
	}
    }
#endif
    default:
	break;
    }
    if (!converted)
	qcoreVariantHandler()->cast(d, t, result, ok);
}

static bool canCast(QVariant::Private *d, QVariant::Type t)
{
    if (d->type == (uint)t)
	return true;

    switch ( t ) {
    case QVariant::Int:
	if (d->type == QVariant::KeySequence)
	    return true;
	break;
    case QVariant::String:
	if (d->type == QVariant::KeySequence || d->type == QVariant::Font || d->type == QVariant::Color)
	    return true;
	break;
    case QVariant::KeySequence:
	return d->type == QVariant::String || d->type == QVariant::Int;
    case QVariant::Font:
	return d->type == QVariant::String;
    case QVariant::Color:
	return d->type == QVariant::String;
    default:
	break;
    }
    return qcoreVariantHandler()->canCast(d, t);
}


const QVariant::Handler qt_gui_variant_handler = {
    construct,
    clear,
    isNull,
#ifndef QT_NO_DATASTREAM
    load,
    save,
#endif
    compare,
    cast,
    canCast
};





/*!
  \fn QVariant::QVariant(const QFont &val)

    Constructs a new variant with a font value, \a val.
*/

/*!
  \fn QVariant::QVariant(const QPixmap &val)

    Constructs a new variant with a pixmap value, \a val.
*/

/*!
  \fn QVariant::QVariant(const QImage &val)

    Constructs a new variant with an image value, \a val.

    Because QImage is explicitly shared, you may need to pass a deep
    copy to the variant using QImage::copy(), e.g. if you intend
    changing the image you've passed later on.
*/

/*!
  \fn QVariant::QVariant(const QBrush &val)

    Constructs a new variant with a brush value, \a val.
*/

/*!
  \fn QVariant::QVariant(const QPoint &val)

    Constructs a new variant with a point value, \a val.
*/

/*!
  \fn QVariant::QVariant(const QRect &val)

    Constructs a new variant with a rect value, \a val.
*/

/*!
  \fn QVariant::QVariant(const QSize &val)

    Constructs a new variant with a size value, \a val.
*/

/*!
  \fn QVariant::QVariant(const QColor &val)

    Constructs a new variant with a color value, \a val.
*/

/*!
  \fn QVariant::QVariant(const QPalette &val)

    Constructs a new variant with a color palette value, \a val.
*/

/*!
  \fn QVariant::QVariant(const QIconSet &val)

    Constructs a new variant with an icon set value, \a val.
*/
/*!
  \fn QVariant::QVariant(const QRegion &val)

    Constructs a new variant with a region value, \a val.
*/

/*!
  \fn QVariant::QVariant(const QBitmap& val)

    Constructs a new variant with a bitmap value, \a val.
*/

/*!
  \fn QVariant::QVariant(const QCursor &val)

    Constructs a new variant with a cursor value, \a val.
*/

/*!
  \fn QVariant::QVariant(const QPointArray &val)

    Constructs a new variant with a point array value, \a val.

    Because QPointArray is explicitly shared, you may need to pass a
    deep copy to the variant using QPointArray::copy(), e.g. if you
    intend changing the point array you've passed later on.
*/

/*!
  \fn QVariant::QVariant(const QKeySequence &val)

    Constructs a new variant with a key sequence value, \a val.
*/

/*!
  \fn QVariant::QVariant(const QPen &val)

    Constructs a new variant with a pen value, \a val.
*/

/*!
  \fn QVariant::QVariant(QSizePolicy val)

    Constructs a new variant with a size policy value, \a val.
*/

/*!
  \fn QFont QVariant::toFont() const

    Returns the variant as a QFont if the variant has type() Font;
    otherwise returns the application's default font.

  \sa asFont()
*/

/*!
    Returns the variant as a QPixmap if the variant has type() Pixmap;
    otherwise returns a null pixmap.

    \sa asPixmap()
*/

/*!
    Returns the variant as a QImage if the variant has type() Image;
    otherwise returns a null image.

    \sa asImage()
*/
/*!
    Returns the variant as a QBrush if the variant has type() Brush;
    otherwise returns a default brush (with all black colors).

    \sa asBrush()
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
  \fn QColor QVariant::toColor() const

    Returns the variant as a QColor if the variant has type() Color;
    otherwise returns an invalid color.

    \sa asColor()
*/

/*!
    Returns the variant as a QPalette if the variant has type()
    Palette; otherwise returns a completely black palette.

    \sa asPalette()
*/

/*!
    Returns the variant as a QColorGroup if the variant has type()
    ColorGroup; otherwise returns an empty color group.
*/
/*!
    Returns the variant as a QIconSet if the variant has type()
    IconSet; otherwise returns an icon set of null pixmaps.

    \sa asIconSet()
*/
/*!
    Returns the variant as a QPointArray if the variant has type()
    PointArray; otherwise returns an empty QPointArray.

    \sa asPointArray()
*/

/*!
    Returns the variant as a QBitmap if the variant has type() Bitmap;
    otherwise returns a null QBitmap.

    \sa asBitmap()
*/

/*!
    Returns the variant as a QRegion if the variant has type() Region;
    otherwise returns an empty QRegion.

    \sa asRegion()
*/
/*!
    Returns the variant as a QCursor if the variant has type() Cursor;
    otherwise returns the default arrow cursor.

    \sa asCursor()
*/

#ifndef QT_NO_ACCEL

/*!
  \fn QKeySequence QVariant::toKeySequence() const

    Returns the variant as a QKeySequence if the variant has type()
    KeySequence, Int or String; otherwise returns an empty key
    sequence.

    Note that not all Ints and Strings are valid key sequences and in
    such cases an empty key sequence will be returned.

    \sa asKeySequence()
*/

#endif // QT_NO_ACCEL

/*!
    Returns the variant as a QPen if the variant has type()
    Pen; otherwise returns an empty QPen.

    \sa asPen()
*/

/*!
    Returns the variant as a QSizePolicy if the variant has type()
    SizePolicy; otherwise returns an undefined (but legal) size
    policy.
*/

QVariant::QVariant(const QPixmap &val)
{
    d = create(Pixmap, &val);
}

QPixmap& QVariant::asPixmap()
{
    return *static_cast<QPixmap *>(castOrDetach(Pixmap));
}

QPixmap QVariant::toPixmap() const
{
    if (d->type != Pixmap)
	return QPixmap();

    return *static_cast<QPixmap *>(d->value.ptr);
}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const QVariant &v)
{
    switch(v.type()) {
    case QVariant::Cursor:
#ifndef QT_NO_CURSOR
	dbg.nospace() << v.toCursor();
#endif
	break;
    case QVariant::Bitmap:
	dbg.nospace() << v.toBitmap();
	break;
    case QVariant::PointArray:
	dbg.nospace() << v.toPointArray();
	break;
    case QVariant::Region:
	dbg.nospace() << v.toRegion();
	break;
    case QVariant::Font:
	dbg.nospace() << v.toFont();
	break;
    case QVariant::Pixmap:
	dbg.nospace() << v.toPixmap();
	break;
    case QVariant::Image:
	dbg.nospace() << v.toImage();
	break;
    case QVariant::Brush:
	dbg.nospace() << v.toBrush();
	break;
    case QVariant::Point:
	dbg.nospace() << v.toPoint();
	break;
    case QVariant::Rect:
	dbg.nospace() << v.toRect();
	break;
    case QVariant::Size:
	dbg.nospace() << v.toSize();
	break;
    case QVariant::Color:
	dbg.nospace() << v.toColor();
	break;
#ifndef QT_NO_PALETTE
    case QVariant::Palette:
	dbg.nospace() << v.toPalette();
	break;
#endif
#ifndef QT_NO_ICONSET
    case QVariant::IconSet:
	dbg.nospace() << v.toIconSet();
	break;
#endif
    case QVariant::SizePolicy:
	dbg.nospace() << v.toSizePolicy();
	break;
#ifndef QT_NO_ACCEL
    case QVariant::KeySequence:
	dbg.nospace() << v.toKeySequence();
	break;
#endif
    case QVariant::Pen:
	dbg.nospace() << v.toPen();
	break;
    default:
        break;
    }
    return dbg.space();
}
#endif


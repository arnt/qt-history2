#include <qvariant.h>

#include "qbitmap.h"
#include "qbrush.h"
#include "qcolor.h"
#include "qcursor.h"
#include "qdatastream.h"
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
extern const QVariant::Handler qt_kernel_variant_handler;


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
#ifndef QT_NO_COMPAT
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
	    qt_kernel_variant_handler.construct(x, v);
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
#ifndef QT_NO_COMPAT
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
	case QVariant::Cursor:
	    x->value.ptr = new QCursor;
	    break;
	default:
	    qt_kernel_variant_handler.construct(x, v);
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
#ifndef QT_NO_COMPAT
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
	qt_kernel_variant_handler.clear(p);
	break;
    }

    p->type = QVariant::Invalid;
    p->is_null = true;
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
#ifndef QT_NO_COMPAT
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
	return qt_kernel_variant_handler.isNull(d);
    }
    return d->is_null;
}


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
#ifndef QT_NO_COMPAT
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
	qt_kernel_variant_handler.load(d, s);
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
#ifndef QT_NO_COMPAT
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
	qt_kernel_variant_handler.save(d, s);
    }
}



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
#ifndef QT_NO_COMPAT
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
    return qt_kernel_variant_handler.compare(a, b);
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
	    *static_cast<int *>(result) = (int)(static_cast<QKeySequence*>(d->value.ptr));
	    converted = true;
	}
	break;
#endif
    case QVariant::Font:
	if (d->type == QVariant::String) {
	    QFont *f = static_cast<QFont *>(result);
	    f->fromString(*static_cast<QString *>(d->value.ptr));
	    converted = true;
	}
	break;
    case QVariant::Color:
	if (d->type == QVariant::String) {
	    static_cast<QColor *>(result)->setNamedColor(*static_cast<QString *>(d->value.ptr));
	    converted = true;
	}
    case QVariant::KeySequence: {
	QKeySequence *seq = static_cast<QKeySequence *>(result);
	switch (d->type) {
	case QVariant::String:
	    *seq = QKeySequence(*static_cast<QString *>(d->value.ptr));
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

    default:
	break;
    }
    if (!converted)
	qt_kernel_variant_handler.cast(d, t, result, ok);
}

static bool canCast(QVariant::Private *d, QVariant::Type t)
{
    if (d->type == t)
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
    return qt_kernel_variant_handler.canCast(d, t);
}


const QVariant::Handler qt_gui_variant_handler = {
    construct,
    clear,
    isNull,
    load,
    save,
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

#define Q_VARIANT_TO(f) \
Q##f QVariant::to##f() const { \
    if ( d->type == f ) \
        return *static_cast<Q##f *>(d->value.ptr); \
    Q##f ret; \
    handler->cast(d, f, &ret, 0); \
    return ret; \
}

Q_VARIANT_TO(Font);
Q_VARIANT_TO(Color);
Q_VARIANT_TO(KeySequence);

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
  \fn QColor QVariant::toColor() const

    Returns the variant as a QColor if the variant has type() Color;
    otherwise returns an invalid color.

    \sa asColor()
*/

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
/*!
    Returns the variant as a QColorGroup if the variant has type()
    ColorGroup; otherwise returns an empty color group.
*/
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
QPen QVariant::toPen() const
{
    if (d->type != Pen)
	return QPen();

    return *static_cast<QPen*>(d->value.ptr);
}

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

/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
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
#include "qicon.h"
#include "qimage.h"
#include "qkeysequence.h"
#include "qpalette.h"
#include "qpen.h"
#include "qpixmap.h"
#include "qpolygon.h"
#include "qregion.h"
#include "qsizepolicy.h"
#include "qtextformat.h"

extern QDataStream &qt_stream_out_qcolorgroup(QDataStream &s, const QColorGroup &g);
extern QDataStream &qt_stream_in_qcolorgroup(QDataStream &s, QColorGroup &g);

Q_CORE_EXPORT const QVariant::Handler *qcoreVariantHandler();

// takes a type, returns the internal void* pointer casted
// to a pointer of the input type
template <typename T>
inline static const T *v_cast(const QCoreVariant::Private *d)
{
    if (sizeof(T) > sizeof(QCoreVariant::Private::Data))
        // this is really a static_cast, but gcc 2.95 complains about it.
        return reinterpret_cast<const T*>(d->data.shared->ptr);
    return reinterpret_cast<const T*>(&d->data.ptr);
}

template <typename T>
inline static T *v_cast(QCoreVariant::Private *d)
{
    if (sizeof(T) > sizeof(QCoreVariant::Private::Data))
        // this is really a static_cast, but gcc 2.95 complains about it.
        return reinterpret_cast<T*>(d->data.shared->ptr);
    return reinterpret_cast<T*>(&d->data.ptr);
}

template <class T>
inline static void v_construct(QCoreVariant::Private *x)
{
    if (sizeof(T) > sizeof(QCoreVariant::Private::Data)) {
        x->data.shared = new QCoreVariant::PrivateShared(new T);
        x->is_shared = true;
    } else {
        new (&x->data.ptr) T;
        x->is_shared = false;
    }
}

template <class T>
inline static void v_construct(QCoreVariant::Private *x, const void *copy)
{
    if (sizeof(T) > sizeof(QCoreVariant::Private::Data)) {
        x->data.shared = new QCoreVariant::PrivateShared(new T(*static_cast<const T *>(copy)));
        x->is_shared = true;
    } else {
        new (&x->data.ptr) T(*static_cast<const T *>(copy));
        x->is_shared = false;
    }
}

static void construct(QCoreVariant::Private *x, const void *copy)
{
    if (copy) {
        switch(x->type) {
        case QVariant::Bitmap:
            v_construct<QBitmap>(x, copy);
            break;
        case QVariant::Region:
            v_construct<QRegion>(x, copy);
            break;
        case QVariant::Polygon:
            v_construct<QPolygon>(x, copy);
            break;
        case QVariant::Font:
            v_construct<QFont>(x, copy);
            break;
        case QVariant::Pixmap:
            v_construct<QPixmap>(x, copy);
            break;
        case QVariant::Image:
            v_construct<QImage>(x, copy);
            break;
        case QVariant::Brush:
            v_construct<QBrush>(x, copy);
            break;
        case QVariant::Color:
            v_construct<QColor>(x, copy);
            break;
#ifndef QT_NO_PALETTE
        case QVariant::Palette:
            v_construct<QPalette>(x, copy);
            break;
#ifdef QT_COMPAT
        case QVariant::ColorGroup:
            v_construct<QColorGroup>(x, copy);
            break;
#endif
#endif
#ifndef QT_NO_ICON
        case QVariant::Icon:
            v_construct<QIcon>(x, copy);
            break;
#endif
        case QVariant::TextLength:
            v_construct<QTextLength>(x, copy);
            break;
#ifndef QT_NO_ACCEL
        case QVariant::KeySequence:
            v_construct<QKeySequence>(x, copy);
            break;
#endif
        case QVariant::Pen:
            v_construct<QPen>(x, copy);
            break;
        case QVariant::SizePolicy:
            v_construct<QSizePolicy>(x, copy);
            break;
        case QVariant::Cursor:
            v_construct<QCursor>(x, copy);
            break;
        default:
            qcoreVariantHandler()->construct(x, copy);
        }
        x->is_null = false;
    } else {
        switch (x->type) {
        case QVariant::Bitmap:
            v_construct<QBitmap>(x);
            break;
        case QVariant::Region:
            v_construct<QRegion>(x);
            break;
        case QVariant::Polygon:
            v_construct<QPolygon>(x);
            break;
        case QVariant::Font:
            v_construct<QFont>(x);
            break;
        case QVariant::Pixmap:
            v_construct<QPixmap>(x);
            break;
        case QVariant::Image:
            v_construct<QImage>(x);
            break;
        case QVariant::Brush:
            v_construct<QBrush>(x);
            break;
        case QVariant::Color:
            v_construct<QColor>(x);
            break;
#ifndef QT_NO_PALETTE
        case QVariant::Palette:
            v_construct<QPalette>(x);
            break;
#ifdef QT_COMPAT
        case QVariant::ColorGroup:
            v_construct<QColorGroup>(x);
            break;
#endif
#endif
#ifndef QT_NO_ICON
        case QVariant::Icon:
            v_construct<QIcon>(x);
            break;
#endif
        case QVariant::TextLength:
            v_construct<QTextLength>(x);
            break;
#ifndef QT_NO_ACCEL
        case QVariant::KeySequence:
            v_construct<QKeySequence>(x);
            break;
#endif
        case QVariant::Pen:
            v_construct<QPen>(x);
            break;
        case QVariant::SizePolicy:
            v_construct<QSizePolicy>(x);
            break;
#ifndef QT_NO_CURSOR
        case QVariant::Cursor:
            v_construct<QCursor>(x);
            break;
#endif
        default:
            qcoreVariantHandler()->construct(x, copy);
        }
    }
}

template <class T>
inline static void v_clear(QCoreVariant::Private *d)
{
    if (sizeof(T) > sizeof(QCoreVariant::Private::Data)) {
        delete v_cast<T>(d);
        delete d->data.shared;
    } else {
        reinterpret_cast<T *>(&d->data.ptr)->~T();
    }
}

static void clear(QCoreVariant::Private *d)
{
    switch (d->type) {
    case QVariant::Bitmap:
        v_clear<QBitmap>(d);
        break;
    case QVariant::Cursor:
        v_clear<QCursor>(d);
        break;
    case QVariant::Region:
        v_clear<QRegion>(d);
        break;
    case QVariant::Polygon:
        v_clear<QPolygon>(d);
        break;
    case QVariant::Font:
        v_clear<QFont>(d);
        break;
    case QVariant::Pixmap:
        v_clear<QPixmap>(d);
        break;
    case QVariant::Image:
        v_clear<QImage>(d);
        break;
    case QVariant::Brush:
        v_clear<QBrush>(d);
        break;
    case QVariant::Color:
        v_clear<QColor>(d);
        break;
#ifndef QT_NO_PALETTE
    case QVariant::Palette:
        v_clear<QPalette>(d);
        break;
#ifdef QT_COMPAT
    case QVariant::ColorGroup:
        v_clear<QColorGroup>(d);
        break;
#endif
#endif
#ifndef QT_NO_ICON
    case QVariant::Icon:
        v_clear<QIcon>(d);
        break;
#endif
    case QVariant::TextLength:
        v_clear<QTextLength>(d);
        break;
    case QVariant::SizePolicy:
        v_clear<QSizePolicy>(d);
        break;
#ifndef QT_NO_ACCEL
    case QVariant::KeySequence:
        v_clear<QKeySequence>(d);
        break;
#endif
    case QVariant::Pen:
        v_clear<QPen>(d);
        break;
    default:
        qcoreVariantHandler()->clear(d);
        return;
    }

    d->type = QVariant::Invalid;
    d->is_null = true;
    d->is_shared = false;
}


static bool isNull(const QVariant::Private *d)
{
    switch(d->type) {
    case QVariant::Bitmap:
        return v_cast<QBitmap>(d)->isNull();
    case QVariant::Region:
        return v_cast<QRegion>(d)->isEmpty();
    case QVariant::Polygon:
        return v_cast<QPolygon>(d)->isEmpty();
    case QVariant::Pixmap:
        return v_cast<QPixmap>(d)->isNull();
    case QVariant::Image:
        return v_cast<QImage>(d)->isNull();
#ifndef QT_NO_ICON
    case QVariant::Icon:
        return v_cast<QIcon>(d)->isNull();
#endif
    case QVariant::TextLength:
    case QVariant::Cursor:
    case QVariant::StringList:
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
        s >> *v_cast<QCursor>(d);
        break;
#endif
#ifndef QT_NO_IMAGEIO
    case QVariant::Bitmap: {
        s >> *v_cast<QBitmap>(d);
        break;
#endif
    case QVariant::Region:
        s >> *v_cast<QRegion>(d);
        break;
    case QVariant::Polygon:
        s >> *v_cast<QPolygon>(d);
        break;
    case QVariant::Font:
        s >> *v_cast<QFont>(d);
        break;
#ifndef QT_NO_IMAGEIO
    case QVariant::Pixmap:
        s >> *v_cast<QPixmap>(d);
        break;
    case QVariant::Image:
        s >> *v_cast<QImage>(d);
        break;
#endif
    case QVariant::Brush:
        s >> *v_cast<QBrush>(d);
        break;
    case QVariant::Color:
        s >> *v_cast<QColor>(d);
        break;
#ifndef QT_NO_PALETTE
    case QVariant::Palette:
        s >> *v_cast<QPalette>(d);
        break;
#ifdef QT_COMPAT
    case QVariant::ColorGroup:
        qt_stream_in_qcolorgroup(s, *v_cast<QColorGroup>(d));
        break;
#endif
#endif
#ifndef QT_NO_ICON
    case QVariant::Icon:
        QPixmap x;
        s >> x;
        *v_cast<QIcon>(d) = QIcon(x);
        break;
    }
#endif
    case QVariant::TextLength: {
        QTextLength x;
        s >> x;
        *v_cast<QTextLength>(d) = x;
        break;
    }
    case QVariant::SizePolicy: {
        int h, v;
        Q_INT8 hfw;
        s >> h >> v >> hfw;
        QSizePolicy *sp = v_cast<QSizePolicy>(d);
        *sp = QSizePolicy(QSizePolicy::SizeType(h), QSizePolicy::SizeType(v));
        sp->setHeightForWidth(bool(hfw));
        break;
    }
#ifndef QT_NO_ACCEL
    case QVariant::KeySequence:
        s >> *v_cast<QKeySequence>(d);
        break;
#endif // QT_NO_ACCEL
    case QVariant::Pen:
        s >> *v_cast<QPen>(d);
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
        s << *v_cast<QCursor>(d);
        break;
    case QVariant::Bitmap:
#ifndef QT_NO_IMAGEIO
        s << *v_cast<QBitmap>(d);
#endif
        break;
    case QVariant::Polygon:
        s << *v_cast<QPolygon>(d);
        break;
    case QVariant::Region:
        s << *v_cast<QRegion>(d);
        break;
    case QVariant::Font:
        s << *v_cast<QFont>(d);
        break;
    case QVariant::Pixmap:
#ifndef QT_NO_IMAGEIO
        s << *v_cast<QPixmap>(d);
#endif
        break;
    case QVariant::Image:
#ifndef QT_NO_IMAGEIO
        s << *v_cast<QImage>(d);
#endif
        break;
    case QVariant::Brush:
        s << *v_cast<QBrush>(d);
        break;
    case QVariant::Color:
        s << *v_cast<QColor>(d);
        break;
#ifndef QT_NO_PALETTE
    case QVariant::Palette:
        s << *v_cast<QPalette>(d);
        break;
#ifdef QT_COMPAT
    case QVariant::ColorGroup:
        qt_stream_out_qcolorgroup(s, *v_cast<QColorGroup>(d));
        break;
#endif
#endif
#ifndef QT_NO_ICON
    case QVariant::Icon:
        //### add stream operator to icon
        s << v_cast<QIcon>(d)->pixmap();
        break;
#endif
    case QVariant::TextLength:
        s << *v_cast<QTextLength>(d);
        break;
    case QVariant::SizePolicy:
        {
            const QSizePolicy *p = v_cast<QSizePolicy>(d);
            s << (int) p->horizontalData() << (int) p->verticalData()
              << (Q_INT8) p->hasHeightForWidth();
        }
        break;
#ifndef QT_NO_ACCEL
    case QVariant::KeySequence:
        s << *v_cast<QKeySequence>(d);
        break;
#endif
    case QVariant::Pen:
        s << *v_cast<QPen>(d);
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
        return v_cast<QCursor>(a)->shape() == v_cast<QCursor>(b)->shape();
#endif
    case QVariant::Bitmap:
        return v_cast<QBitmap>(a)->serialNumber()
            == v_cast<QBitmap>(b)->serialNumber();
    case QVariant::Polygon:
        return *v_cast<QPolygon>(a) == *v_cast<QPolygon>(b);
    case QVariant::Region:
        return *v_cast<QRegion>(a) == *v_cast<QRegion>(b);
    case QVariant::Font:
        return *v_cast<QFont>(a) == *v_cast<QFont>(b);
    case QVariant::Pixmap:
        return v_cast<QPixmap>(a)->serialNumber() == v_cast<QPixmap>(b)->serialNumber();
    case QVariant::Image:
        return *v_cast<QImage>(a) == *v_cast<QImage>(b);
    case QVariant::Brush:
        return *v_cast<QBrush>(a) == *v_cast<QBrush>(b);
    case QVariant::Color:
        return *v_cast<QColor>(a) == *v_cast<QColor>(b);
#ifndef QT_NO_PALETTE
    case QVariant::Palette:
        return *v_cast<QPalette>(a) == *v_cast<QPalette>(b);
#ifdef QT_COMPAT
    case QVariant::ColorGroup:
        return *v_cast<QColorGroup>(a) == *v_cast<QColorGroup>(b);
#endif
#endif
#ifndef QT_NO_ICON
    case QVariant::Icon:
        return v_cast<QIcon>(a)->pixmap().serialNumber()
               == v_cast<QIcon>(b)->pixmap().serialNumber();
#endif
    case QVariant::TextLength:
        return *v_cast<QTextLength>(a) == *v_cast<QTextLength>(b);
    case QVariant::SizePolicy:
        return *v_cast<QSizePolicy>(a) == *v_cast<QSizePolicy>(b);
#ifndef QT_NO_ACCEL
    case QVariant::KeySequence:
        return *v_cast<QKeySequence>(a) == *v_cast<QKeySequence>(b);
#endif
    case QVariant::Pen:
        return *v_cast<QPen>(a) == *v_cast<QPen>(b);
    default:
        break;
    }
    return qcoreVariantHandler()->compare(a, b);
}



static void cast(const QCoreVariant::Private *d, QVariant::Type t,
                 void *result, bool *ok)
{
    bool converted = false;
    switch (t) {
    case QVariant::String: {
        QString *str = static_cast<QString *>(result);
        switch (d->type) {
#ifndef QT_NO_ACCEL
        case QVariant::KeySequence:
            *str = QString(*v_cast<QKeySequence>(d));
            converted = true;
            break;
#endif
        case QVariant::Font:
            *str = v_cast<QFont>(d)->toString();
            converted = true;
            break;
        case QVariant::Color:
            *str = v_cast<QColor>(d)->name();
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
            *static_cast<int *>(result) = (int)(*(v_cast<QKeySequence>(d)));
            converted = true;
        }
        break;
#endif
    case QVariant::Font:
        if (d->type == QVariant::String) {
            QFont *f = static_cast<QFont *>(result);
            f->fromString(*v_cast<QString>(d));
            converted = true;
        }
        break;
    case QVariant::Color:
        if (d->type == QVariant::String) {
            static_cast<QColor *>(result)->setNamedColor(*v_cast<QString>(d));
            converted = true;
        }
        break;
#ifndef QT_NO_ACCEL
    case QVariant::KeySequence: {
        QKeySequence *seq = static_cast<QKeySequence *>(result);
        switch (d->type) {
        case QVariant::String:
            *seq = QKeySequence(*v_cast<QString>(d));
            converted = true;
            break;
        case QVariant::Int:
            *seq = QKeySequence(d->data.i);
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

static bool canCast(const QVariant::Private *d, QVariant::Type t)
{
    if (d->type == uint(t))
        return true;

    switch (t) {
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
    \class QVariant
    \brief The QVariant class is a safe generic container for the most common Qt data types.

    \ingroup objectmodel
    \ingroup misc
    \mainclass

    This class is derived from QCoreVariant. It includes all the types
    from QCoreVariant and additional types which make sense for GUI
    applications.
*/

/*!
    \fn QVariant::QVariant()

    Constructs an invalid variant.
*/

/*!
    \fn QVariant::QVariant(QObject *object)

    \internal

    Constructs a variant that stores the given \a object.
*/

/*!
    \fn QVariant::QVariant(int typeOrUserType, const void *v);

    \internal

    Constructs a variant of type \a typeOrUserType, and initializes
    with \a v if \a v is not 0.
*/

/*!
    \fn QVariant::QVariant(Type type)

    \internal

    Constructs a variant of type \a type with no value.
*/

/*!
    \fn QVariant::QVariant(const QVariant &other)

    Constructs a variant with the value of \a other.
*/

/*!
    \fn QVariant::QVariant(const QCoreVariant &other)

    Constructs a variant with the value of \a other.
*/

/*!
  \fn QVariant::QVariant(const QFont &val)

    Constructs a new variant with a font value of \a val.
*/

/*!
  \fn QVariant::QVariant(const QPixmap &val)

    Constructs a new variant with a pixmap value of \a val.
*/

/*!
  \fn QVariant::QVariant(const QImage &val)

    Constructs a new variant with an image value of \a val.

    Because QImage is explicitly shared, you may need to pass a deep
    copy to the variant using QImage::copy(), e.g. if you intend
    changing the image you've passed later on.
*/

/*!
  \fn QVariant::QVariant(const QBrush &val)

    Constructs a new variant with a brush value of \a val.
*/

/*!
  \fn QVariant::QVariant(const QSizePolicy &val)

    Constructs a new variant with a size policy value of \a val.
*/

/*!
  \fn QVariant::QVariant(const QColor &val)

    Constructs a new variant with a color value of \a val.
*/

/*!
  \fn QVariant::QVariant(const QPalette &val)

    Constructs a new variant with a color palette value of \a val.
*/

/*!
  \fn QVariant::QVariant(const QIcon &val)

    Constructs a new variant with an icon set value of \a val.
*/
/*!
  \fn QVariant::QVariant(const QRegion &val)

    Constructs a new variant with a region value of \a val.
*/

/*!
  \fn QVariant::QVariant(const QBitmap& val)

    Constructs a new variant with a bitmap value of \a val.
*/

/*!
  \fn QVariant::QVariant(const QCursor &val)

    Constructs a new variant with a cursor value of \a val.
*/

/*!
  \fn QVariant::QVariant(const QPolygon &val)

    Constructs a new variant with a point array value of \a val.

    Because QPolygon is explicitly shared, you may need to pass a
    deep copy to the variant using QPolygon::copy(), e.g. if you
    intend changing the point array you've passed later on.
*/

/*!
  \fn QVariant::QVariant(const QKeySequence &val)

    Constructs a new variant with a key sequence value of \a val.
*/

/*!
  \fn QVariant::QVariant(const QPen &val)

    Constructs a new variant with a pen value of \a val.
*/

/*!
    \fn QVariant::QVariant(int val)

    Constructs a new variant with an integer value of \a val.
*/

/*!
    \fn QVariant::QVariant(uint val)

    Constructs a new variant with an unsigned integer value of \a val.
*/

/*!
    \fn QVariant::QVariant(Q_LONGLONG val)

    Constructs a new variant with a long integer value of \a val.
*/

/*!
    \fn QVariant::QVariant(Q_ULONGLONG val)

    Constructs a new variant with an unsigned long integer value of \a val.
*/

/*!
    \fn QVariant::QVariant(double val)

    Constructs a new variant with a double value of \a val.
*/

/*!
    \fn QVariant::QVariant(bool val)

    Constructs a new variant with a boolean value of \a val.
*/

/*!
    \fn QVariant::QVariant(const char *val)

    Constructs a new variant with a byte array value of \a val.
*/

/*!
    \fn QVariant::QVariant(const QByteArray &val)

    Constructs a new variant with a byte array value of \a val.
*/

/*!
    \fn QVariant::QVariant(const QBitArray &val)

    Constructs a new variant with a bit array value of \a val.
*/

/*!
    \fn QVariant::QVariant(const QString &val)

    Constructs a new variant with a string value of \a val.
*/

/*!
    \fn QVariant::QVariant(const QLatin1String &val)

    Constructs a new variant with a string value of \a val.
*/

/*!
    \fn QVariant::QVariant(const QStringList &val)

    Constructs a new variant with a string list value of \a val.
*/

/*!
    \fn QVariant::QVariant(const QDate &val)

    Constructs a new variant with a date value of \a val.
*/

/*!
    \fn QVariant::QVariant(const QTime &val)

    Constructs a new variant with a time value of \a val.
*/

/*!
    \fn QVariant::QVariant(const QDateTime &val)

    Constructs a new variant with a date/time value of \a val.
*/

/*!
    \fn QVariant::QVariant(const QList<QVariant> &val)

    Constructs a new variant that holds the list of variants given in
    \a val.
*/

/*!
    \fn QVariant::QVariant(const QList<QCoreVariant> &val)

    Constructs a new variant that holds the list of variants given in
    \a val.
*/

/*!
    \fn QVariant::QVariant(const QMap<QString, QVariant> &val)

    Constructs a new variant that holds a map whose keys are strings
    and whose values are variants, as given in \a val.
*/

/*!
    \fn QVariant::QVariant(const QMap<QString, QCoreVariant> &val)

    Constructs a new variant that holds a map whose keys are strings
    and whose values are variants, as given in \a val.
*/

/*! \internal
 */
static const QVariant::Handler *qRegisterGuiVariantHandler(const QVariant::Handler *&handler)
{
    handler = &qt_gui_variant_handler;
    return handler;
}

void QVariant::create(int type, const void *copy)
{
    static const Handler *h = qRegisterGuiVariantHandler(handler);
    Q_UNUSED(h);
    QCoreVariant::create(type, copy);
}


QVariant::QVariant(int typeOrUserType, const void *v)
{ create(typeOrUserType, v); d.is_null = false; }
QVariant::QVariant(const QFont &val) { create(Font, &val); }
QVariant::QVariant(const QPixmap &val) { create(Pixmap, &val); }

QVariant::QVariant(const QImage &val) { create(Image, &val); }
QVariant::QVariant(const QBrush &val) { create(Brush, &val); }
QVariant::QVariant(const QColor &val) { create(Color, &val); }
#ifndef QT_NO_PALETTE
QVariant::QVariant(const QPalette &val) { create(Palette, &val); }
#ifdef QT_COMPAT
/*!
    QVariant's don't store color groups directly; store and retrieve a
    QPalette instead.
*/
QVariant::QVariant(const QColorGroup &val) { create(ColorGroup, &val); }
#endif
#endif //QT_NO_PALETTE
#ifndef QT_NO_ICON
QVariant::QVariant(const QIcon &val) { create(Icon, &val); }
#endif //QT_NO_ICON
QVariant::QVariant(const QTextLength &val) { create(TextLength, &val); }
QVariant::QVariant(const QPolygon &val) { create(Polygon, &val); }
QVariant::QVariant(const QRegion &val) { create(Region, &val); }
QVariant::QVariant(const QBitmap& val) { create(Bitmap, &val); }
#ifndef QT_NO_CURSOR
QVariant::QVariant(const QCursor &val) { create(Cursor, &val); }
#endif
#ifndef QT_NO_ACCEL
QVariant::QVariant(const QKeySequence &val) { create(KeySequence, &val); }
#endif
QVariant::QVariant(const QPen &val) { create(Pen, &val); }
QVariant::QVariant(const QSizePolicy &val) { create(SizePolicy, &val); }




/*!
  \fn QFont QVariant::toFont() const

    Returns the variant as a QFont if the variant has type() Font;
    otherwise returns the application's default font.
*/

/*!
    \fn QPixmap QVariant::toPixmap() const

    Returns the variant as a QPixmap if the variant has type() Pixmap;
    otherwise returns a null pixmap.
*/

/*!
    \fn const QImage QVariant::toImage() const

    Returns the variant as a QImage if the variant has type() Image;
    otherwise returns a null image.
*/

/*!
    \fn QBrush QVariant::toBrush() const

    Returns the variant as a QBrush if the variant has type() Brush;
    otherwise returns a default brush (with all black colors).
*/

/*!
    \fn QColor QVariant::toColor() const

    Returns the variant as a QColor if the variant has type() Color;
    otherwise returns an invalid color.
*/

/*!
    \fn QPalette QVariant::toPalette() const

    Returns the variant as a QPalette if the variant has type()
    Palette; otherwise returns a copy of the application's default
    palette.
*/

/*!
    \fn QIcon QVariant::toIcon() const

    Returns the variant as a QIcon if the variant has type()
    Icon; otherwise returns a null QIcon.
*/

/*!
    \fn QPolygon QVariant::toPolygon() const

    Returns the variant as a QPolygon if the variant has type()
    Polygon; otherwise returns a null QPolygon.
*/

/*!
    \fn QBitmap QVariant::toBitmap() const

    Returns the variant as a QBitmap if the variant has type()
    Bitmap; otherwise returns a null QBitmap.
*/

/*!
    \fn QRegion QVariant::toRegion() const

    Returns the variant as a QRegion if the variant has type()
    Region; otherwise returns an empty QRegion.
*/

/*!
    \fn QCursor QVariant::toCursor() const

    Returns the variant as a QCursor if the variant has type()
    Cursor; otherwise returns the default arrow cursor.
*/

/*!
    \fn QPen QVariant::toPen() const

    Returns the variant as a QPen if the variant has type()
    Pen; otherwise returns a default pen that will draw 1-pixel wide
    solid black lines.
*/

/*!
    \fn QSizePolicy QVariant::toSizePolicy() const

    Returns the variant as a QSizePolicy if the variant has type()
    SizePolicy; otherwise returns a minimally initialized QSizePolicy.
*/

#ifndef QT_NO_ACCEL

/*!
  \fn QKeySequence QVariant::toKeySequence() const

    Returns the variant as a QKeySequence if the variant has type()
    KeySequence, Int or String; otherwise returns an empty key
    sequence.

    Note that not all Ints and Strings are valid key sequences and in
    such cases an empty key sequence will be returned.
*/

#endif // QT_NO_ACCEL

const QImage QVariant::toImage() const
{
    if (d.type != Image)
        return QImage();

    return *v_cast<QImage>(&d);
}

QBrush QVariant::toBrush() const
{
    if (d.type != Brush)
        return QBrush();

    return *v_cast<QBrush>(&d);
}


#ifndef QT_NO_PALETTE
QPalette QVariant::toPalette() const
{
    if (d.type != Palette)
        return QPalette();

    return *v_cast<QPalette>(&d);
}

#ifdef QT_COMPAT
/*!
    QVariant's don't store color groups directly; store and retrieve a
    QPalette instead. See toPalette().
*/
QColorGroup QVariant::toColorGroup() const
{
    if (d.type != ColorGroup)
        return QColorGroup();

    return *v_cast<QColorGroup>(&d);
}
#endif
#endif //QT_NO_PALETTE

QPen QVariant::toPen() const
{
    if (d.type != Pen)
        return QPen();

    return *v_cast<QPen>(&d);
}

QSizePolicy QVariant::toSizePolicy() const
{
    if (d.type != SizePolicy)
        return QSizePolicy();

    return *v_cast<QSizePolicy>(&d);
}

#ifndef QT_NO_CURSOR
QCursor QVariant::toCursor() const
{
    if (d.type != Cursor)
        return QCursor();

    return *v_cast<QCursor>(&d);
}
#endif

QRegion QVariant::toRegion() const
{
    if (d.type != Region)
        return QRegion();

    return *v_cast<QRegion>(&d);
}

QBitmap QVariant::toBitmap() const
{
    if (d.type != Bitmap)
        return QBitmap();

    return *v_cast<QBitmap>(&d);

}

const QPolygon QVariant::toPolygon() const
{
    if (d.type != Polygon)
        return QPolygon();

    return *v_cast<QPolygon>(&d);
}

#ifndef QT_NO_ICON
QIcon QVariant::toIcon() const
{
    if (d.type != Icon)
        return QIcon();

    return *v_cast<QIcon>(&d);
}
#ifdef QT_COMPAT
/*!
    Use toIcon() instead.
*/
QIcon QVariant::toIconSet() const { return toIcon(); }
#endif

#endif //QT_NO_ICON

QTextLength QVariant::toTextLength() const
{
    if (d.type != TextLength)
        return QTextLength();

    return *v_cast<QTextLength>(&d);
}


#define Q_VARIANT_TO(f) \
Q##f QVariant::to##f() const { \
    if (d.type == f) \
        return *v_cast<Q##f>(&d); \
    static const Handler *h = qRegisterGuiVariantHandler(handler); \
    Q_UNUSED(h); \
    Q##f ret; \
    handler->cast(&d, f, &ret, 0); \
    return ret; \
}

Q_VARIANT_TO(Font);
Q_VARIANT_TO(Color);
#ifndef QT_NO_ACCEL
Q_VARIANT_TO(KeySequence);
#endif

QPixmap QVariant::toPixmap() const
{
    if (d.type != Pixmap)
        return QPixmap();

    return *v_cast<QPixmap>(&d);
}

#ifndef QT_NO_DEBUG_OUTPUT
QDebug operator<<(QDebug dbg, const QVariant &v)
{
#ifndef Q_NO_STREAMING_DEBUG
    switch(v.type()) {
    case QVariant::Cursor:
#ifndef QT_NO_CURSOR
        dbg.nospace() << v.toCursor();
#endif
        break;
    case QVariant::Bitmap:
//        dbg.nospace() << v.toBitmap(); //FIXME
        break;
    case QVariant::Polygon:
        dbg.nospace() << v.toPolygon();
        break;
    case QVariant::Region:
        dbg.nospace() << v.toRegion();
        break;
    case QVariant::Font:
//        dbg.nospace() << v.toFont();  //FIXME
        break;
    case QVariant::Pixmap:
//        dbg.nospace() << v.toPixmap(); //FIXME
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
#ifndef QT_NO_ICON
    case QVariant::Icon:
        dbg.nospace() << v.toIcon();
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
        dbg.nospace() << static_cast<QCoreVariant>(v);
        break;
    }
    return dbg.space();
#else
    qWarning("This compiler doesn't support the streaming of QDebug");
    return dbg;
    Q_UNUSED(v);
#endif
}


#if defined Q_CC_MSVC && _MSC_VER < 1300

template<> QFont QVariant_to_helper<QFont>(const QCoreVariant &v, const QFont*)
{ return static_cast<const QVariant &>(v).toFont(); }
template<> QPixmap QVariant_to_helper<QPixmap>(const QCoreVariant &v, const QPixmap*)
{ return static_cast<const QVariant &>(v).toPixmap(); }
template<> QImage QVariant_to_helper<QImage>(const QCoreVariant &v, const QImage*)
{ return static_cast<const QVariant &>(v).toImage(); }
template<> QBrush QVariant_to_helper<QBrush>(const QCoreVariant &v, const QBrush*)
{ return static_cast<const QVariant &>(v).toBrush(); }
template<> QColor QVariant_to_helper<QColor>(const QCoreVariant &v, const QColor*)
{ return static_cast<const QVariant &>(v).toColor(); }
template<> QPalette QVariant_to_helper<QPalette>(const QCoreVariant &v, const QPalette*)
{ return static_cast<const QVariant &>(v).toPalette(); }
template<> QIcon QVariant_to_helper<QIcon>(const QCoreVariant &v, const QIcon*)
{ return static_cast<const QVariant &>(v).toIcon(); }
template<> QTextLength QVariant_to_helper<QTextLength>(const QCoreVariant &v, const QTextLength*)
{ return static_cast<const QVariant &>(v).toTextLength(); }
template<> QPolygon QVariant_to_helper<QPolygon>(const QCoreVariant &v, const QPolygon*)
{ return static_cast<const QVariant &>(v).toPolygon(); }
template<> QBitmap QVariant_to_helper<QBitmap>(const QCoreVariant &v, const QBitmap*)
{ return static_cast<const QVariant &>(v).toBitmap(); }
template<> QRegion QVariant_to_helper<QRegion>(const QCoreVariant &v, const QRegion*)
{ return static_cast<const QVariant &>(v).toRegion(); }
#ifndef QT_NO_CURSOR
template<> QCursor QVariant_to_helper<QCursor>(const QCoreVariant &v, const QCursor*)
{ return static_cast<const QVariant &>(v).toCursor(); }
#endif
#ifndef QT_NO_ACCEL
template<> QKeySequence QVariant_to_helper<QKeySequence>(const QCoreVariant &v, const QKeySequence*)
{ return static_cast<const QVariant &>(v).toKeySequence(); }
#endif
template<> QPen QVariant_to_helper<QPen>(const QCoreVariant &v, const QPen*)
{ return static_cast<const QVariant &>(v).toPen(); }
template<> QSizePolicy QVariant_to_helper<QSizePolicy>(const QCoreVariant &v, const QSizePolicy*)
{ return static_cast<const QVariant &>(v).toSizePolicy(); }

#else

template<> QFont QVariant_to<QFont>(const QCoreVariant &v)
{ return static_cast<const QVariant &>(v).toFont(); }
template<> QPixmap QVariant_to<QPixmap>(const QCoreVariant &v)
{ return static_cast<const QVariant &>(v).toPixmap(); }
template<> QImage QVariant_to<QImage>(const QCoreVariant &v)
{ return static_cast<const QVariant &>(v).toImage(); }
template<> QBrush QVariant_to<QBrush>(const QCoreVariant &v)
{ return static_cast<const QVariant &>(v).toBrush(); }
template<> QColor QVariant_to<QColor>(const QCoreVariant &v)
{ return static_cast<const QVariant &>(v).toColor(); }
template<> QPalette QVariant_to<QPalette>(const QCoreVariant &v)
{ return static_cast<const QVariant &>(v).toPalette(); }
template<> QIcon QVariant_to<QIcon>(const QCoreVariant &v)
{ return static_cast<const QVariant &>(v).toIcon(); }
template<> QTextLength QVariant_to<QTextLength>(const QCoreVariant &v)
{ return static_cast<const QVariant &>(v).toTextLength(); }
template<> QPolygon QVariant_to<QPolygon>(const QCoreVariant &v)
{ return static_cast<const QVariant &>(v).toPolygon(); }
template<> QBitmap QVariant_to<QBitmap>(const QCoreVariant &v)
{ return static_cast<const QVariant &>(v).toBitmap(); }
template<> QRegion QVariant_to<QRegion>(const QCoreVariant &v)
{ return static_cast<const QVariant &>(v).toRegion(); }
#ifndef QT_NO_CURSOR
template<> QCursor QVariant_to<QCursor>(const QCoreVariant &v)
{ return static_cast<const QVariant &>(v).toCursor(); }
#endif
#ifndef QT_NO_ACCEL
template<> QKeySequence QVariant_to<QKeySequence>(const QCoreVariant &v)
{ return static_cast<const QVariant &>(v).toKeySequence(); }
#endif
template<> QPen QVariant_to<QPen>(const QCoreVariant &v)
{ return static_cast<const QVariant &>(v).toPen(); }
template<> QSizePolicy QVariant_to<QSizePolicy>(const QCoreVariant &v)
{ return static_cast<const QVariant &>(v).toSizePolicy(); }
#endif


#endif

/*!
    \fn QVariant::QVariant(bool b, int dummy)

    Use the single-argument overload instead.
*/

/*!
    \fn QFont& QVariant::asFont()

    Use toFont() instead.
*/

/*!
    \fn QImage& QVariant::asImage()

    Use toImage() instead.
*/

/*!
    \fn QBrush& QVariant::asBrush()

    Use toBrush() instead.
*/

/*!
    \fn QColor& QVariant::asColor()

    Use toColor() instead.
*/

/*!
    \fn QPalette& QVariant::asPalette()

    Use toPalette() instead.
*/

/*!
    \fn QColorGroup& QVariant::asColorGroup()

    QVariant's don't store color groups directly; store and retrieve a
    QPalette instead.
*/

/*!
    \fn QIcon& QVariant::asIconSet()

    Use toIcon() instead.
*/

/*!
    \fn QPolygon& QVariant::asPointArray()

    Use toPolygon() instead.
*/

/*!
    \fn QBitmap& QVariant::asBitmap()

    Use toBitmap() instead.
*/

/*!
    \fn QRegion& QVariant::asRegion()

    Use toRegion() instead.
*/

/*!
    \fn QCursor& QVariant::asCursor()

    Use toCursor() instead.
*/

/*!
    \fn QKeySequence& QVariant::asKeySequence()

    Use toKeySequence() instead.
*/

/*!
    \fn QPen& QVariant::asPen()

    Use toPen() instead.
*/

/*!
    \fn QSizePolicy& QVariant::asSizePolicy()

    Use toSizePolicy() instead.
*/

/*!
    \fn QPixmap& QVariant::asPixmap()

    Use toPixmap() instead.
*/


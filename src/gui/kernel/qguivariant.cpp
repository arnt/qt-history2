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

#include "qvariant.h"

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

#include "private/qvariant_p.h"

#ifdef QT3_SUPPORT
extern QDataStream &qt_stream_out_qcolorgroup(QDataStream &s, const QColorGroup &g);
extern QDataStream &qt_stream_in_qcolorgroup(QDataStream &s, QColorGroup &g);
#endif

Q_CORE_EXPORT const QVariant::Handler *qcoreVariantHandler();

static void construct(QVariant::Private *x, const void *copy)
{
    switch (x->type) {
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
#ifdef QT3_SUPPORT
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
    case QVariant::TextFormat:
        v_construct<QTextFormat>(x, copy);
        break;
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
        return;
    }
    x->is_null = !copy;
}

static void clear(QVariant::Private *d)
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
#ifdef QT3_SUPPORT
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
    case QVariant::TextFormat:
        v_clear<QTextFormat>(d);
        break;
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
    case QVariant::TextFormat:
    case QVariant::TextLength:
    case QVariant::Cursor:
    case QVariant::StringList:
    case QVariant::Font:
    case QVariant::Brush:
    case QVariant::Color:
#ifndef QT_NO_PALETTE
    case QVariant::Palette:
#ifdef QT3_SUPPORT
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
#ifdef QT3_SUPPORT
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
    case QVariant::TextFormat: {
        QTextFormat x;
        s >> x;
        *v_cast<QTextFormat>(d) = x;
        break;
    }
    case QVariant::TextLength: {
        QTextLength x;
        s >> x;
        *v_cast<QTextLength>(d) = x;
        break;
    }
    case QVariant::SizePolicy: {
        int h, v;
        qint8 hfw;
        s >> h >> v >> hfw;
        QSizePolicy *sp = v_cast<QSizePolicy>(d);
        *sp = QSizePolicy(QSizePolicy::Policy(h), QSizePolicy::Policy(v));
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
#ifdef QT3_SUPPORT
    case QVariant::ColorGroup:
        qt_stream_out_qcolorgroup(s, *v_cast<QColorGroup>(d));
        break;
#endif
#endif
#ifndef QT_NO_ICON
    case QVariant::Icon:
        //### add stream operator to icon
        s << v_cast<QIcon>(d)->pixmap(QSize(22, 22)); //FIXME
        break;
#endif
    case QVariant::TextFormat:
        s << *v_cast<QTextFormat>(d);
        break;
    case QVariant::TextLength:
        s << *v_cast<QTextLength>(d);
        break;
    case QVariant::SizePolicy:
        {
            const QSizePolicy *p = v_cast<QSizePolicy>(d);
            s << (int) p->horizontalPolicy() << (int) p->verticalPolicy()
              << (qint8) p->hasHeightForWidth();
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
#ifdef QT3_SUPPORT
    case QVariant::ColorGroup:
        return *v_cast<QColorGroup>(a) == *v_cast<QColorGroup>(b);
#endif
#endif
#ifndef QT_NO_ICON
    case QVariant::Icon:
        return false; // #### FIXME
#endif
    case QVariant::TextFormat:
        return *v_cast<QTextFormat>(a) == *v_cast<QTextFormat>(b);
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



static bool convert(const QVariant::Private *d, QVariant::Type t,
                 void *result, bool *ok)
{
    switch (t) {
    case QVariant::ByteArray:
        if (d->type == QVariant::Color) {
            *static_cast<QByteArray *>(result) = v_cast<QColor>(d)->name().toLatin1();
            return true;
        }
        break;
    case QVariant::String: {
        QString *str = static_cast<QString *>(result);
        switch (d->type) {
#ifndef QT_NO_ACCEL
        case QVariant::KeySequence:
            *str = QString(*v_cast<QKeySequence>(d));
            return true;
#endif
        case QVariant::Font:
            *str = v_cast<QFont>(d)->toString();
            return true;
        case QVariant::Color:
            *str = v_cast<QColor>(d)->name();
            return true;
        default:
            break;
        }
        break;
    }
    case QVariant::Pixmap:
        if (d->type == QVariant::Image) {
            *static_cast<QPixmap *>(result) = QPixmap::fromImage(*v_cast<QImage>(d));
            return true;
        } else if (d->type == QVariant::Bitmap) {
            *static_cast<QPixmap *>(result) = *v_cast<QBitmap>(d);
            return true;
        }
        break;
    case QVariant::Image:
        if (d->type == QVariant::Pixmap) {
            *static_cast<QImage *>(result) = v_cast<QPixmap>(d)->toImage();
            return true;
        } else if (d->type == QVariant::Bitmap) {
            *static_cast<QImage *>(result) = v_cast<QBitmap>(d)->toImage();
            return true;
        }
        break;
    case QVariant::Bitmap:
        if (d->type == QVariant::Pixmap) {
            *static_cast<QBitmap *>(result) = *v_cast<QPixmap>(d);
            return true;
        } else if (d->type == QVariant::Image) {
            *static_cast<QBitmap *>(result) = QBitmap::fromImage(*v_cast<QImage>(d));
            return true;
        }
        break;
#ifndef QT_NO_ACCEL
    case QVariant::Int:
        if (d->type == QVariant::KeySequence) {
            *static_cast<int *>(result) = (int)(*(v_cast<QKeySequence>(d)));
            return true;
        }
        break;
#endif
    case QVariant::Font:
        if (d->type == QVariant::String) {
            QFont *f = static_cast<QFont *>(result);
            f->fromString(*v_cast<QString>(d));
            return true;
        }
        break;
    case QVariant::Color:
        if (d->type == QVariant::String) {
            static_cast<QColor *>(result)->setNamedColor(*v_cast<QString>(d));
            return true;
        } else if (d->type == QVariant::ByteArray) {
            static_cast<QColor *>(result)->setNamedColor(QString::fromLatin1(
                                *v_cast<QByteArray>(d)));
            return true;
        }
        break;
#ifndef QT_NO_ACCEL
    case QVariant::KeySequence: {
        QKeySequence *seq = static_cast<QKeySequence *>(result);
        switch (d->type) {
        case QVariant::String:
            *seq = QKeySequence(*v_cast<QString>(d));
            return true;
        case QVariant::Int:
            *seq = QKeySequence(d->data.i);
            return true;
        default:
            break;
        }
    }
#endif
    default:
        break;
    }
    return qcoreVariantHandler()->convert(d, t, result, ok);
}

static bool canConvert(const QVariant::Private *d, QVariant::Type t)
{
    if (d->type == uint(t))
        return true;

    switch (t) {
    case QVariant::Int:
        if (d->type == QVariant::KeySequence)
            return true;
        break;
    case QVariant::Image:
        return d->type == QVariant::Pixmap || d->type == QVariant::Bitmap;
    case QVariant::Pixmap:
        return d->type == QVariant::Image || d->type == QVariant::Bitmap;
    case QVariant::Bitmap:
        return d->type == QVariant::Pixmap || d->type == QVariant::Image;
    case QVariant::ByteArray:
        if (d->type == QVariant::Color)
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
        return d->type == QVariant::String || d->type == QVariant::ByteArray;
    default:
        break;
    }
    return qcoreVariantHandler()->canConvert(d, t);
}

#if !defined(QT_NO_DEBUG_STREAM) && !defined(Q_BROKEN_DEBUG_STREAM)
static void streamDebug(QDebug dbg, const QVariant &v)
{
    switch(v.type()) {
    case QVariant::Cursor:
#ifndef QT_NO_CURSOR
//        dbg.nospace() << qvariant_cast<QCursor>(v); //FIXME
#endif
        break;
    case QVariant::Bitmap:
//        dbg.nospace() << qvariant_cast<QBitmap>(v); //FIXME
        break;
    case QVariant::Polygon:
        dbg.nospace() << qvariant_cast<QPolygon>(v);
        break;
    case QVariant::Region:
        dbg.nospace() << qvariant_cast<QRegion>(v);
        break;
    case QVariant::Font:
//        dbg.nospace() << qvariant_cast<QFont>(v);  //FIXME
        break;
    case QVariant::Pixmap:
//        dbg.nospace() << qvariant_cast<QPixmap>(v); //FIXME
        break;
    case QVariant::Image:
//        dbg.nospace() << qvariant_cast<QImage>(v); //FIXME
        break;
    case QVariant::Brush:
        dbg.nospace() << qvariant_cast<QBrush>(v);
        break;
    case QVariant::Color:
        dbg.nospace() << qvariant_cast<QColor>(v);
        break;
#ifndef QT_NO_PALETTE
    case QVariant::Palette:
//        dbg.nospace() << qvariant_cast<QPalette>(v); //FIXME
        break;
#endif
#ifndef QT_NO_ICON
    case QVariant::Icon:
//        dbg.nospace() << qvariant_cast<QIcon>(v); // FIXME
        break;
#endif
    case QVariant::SizePolicy:
//        dbg.nospace() << qvariant_cast<QSizePolicy>(v); //FIXME
        break;
#ifndef QT_NO_ACCEL
    case QVariant::KeySequence:
        dbg.nospace() << qvariant_cast<QKeySequence>(v);
        break;
#endif
    case QVariant::Pen:
//        dbg.nospace() << qvariant_cast<QPen>(v); //FIXME
        break;
    default:
        qcoreVariantHandler()->debugStream(dbg, v);
        break;
    }
}
#endif

const QVariant::Handler qt_gui_variant_handler = {
    construct,
    clear,
    isNull,
#ifndef QT_NO_DATASTREAM
    load,
    save,
#endif
    compare,
    convert,
    canConvert,
#if !defined(QT_NO_DEBUG_STREAM) && !defined(Q_BROKEN_DEBUG_STREAM)
    streamDebug
#else
    0
#endif
};



/*! \internal
 */
static const QVariant::Handler *qRegisterGuiVariantHandler(const QVariant::Handler *&handler)
{
    handler = &qt_gui_variant_handler;
    return handler;
}

bool qRegisterGuiVariant()
{
    static const QVariant::Handler *h = qRegisterGuiVariantHandler(QVariant::handler);
    Q_UNUSED(h);
    return true;
}



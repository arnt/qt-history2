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

#include "qkernelvariant.h"
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
#ifndef QT_NO_COMPAT
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

class Q_GUI_EXPORT QVariant : public QKernelVariant
{
 public:
    inline QVariant();

    inline QVariant(const QFont &font);
    inline QVariant(const QPixmap &pixmap);
    inline QVariant(const QImage &image);
    inline QVariant(const QBrush &brush);
    inline QVariant(const QColor &color);
#ifndef QT_NO_PALETTE
    inline QVariant(const QPalette &palette);
#ifndef QT_NO_COMPAT
    inline QVariant(const QColorGroup &cg);
#endif // QT_NO_COMPAT
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

    // Copied from qkernelvariant.h
    inline QVariant(Type type, void *v = 0);
    inline QVariant(const QKernelVariant &other);
    inline QVariant(const QVariant &other);

    inline QVariant(int i);
    inline QVariant(uint ui);
    inline QVariant(Q_LLONG ll);
    inline QVariant(Q_ULLONG ull);
    inline QVariant(bool b);
    inline QVariant(double d);

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
    inline QVariant(const QList<QKernelVariant> &list);
    inline QVariant(const QMap<QString,QKernelVariant> &map);
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
#ifndef QT_NO_COMPAT
    QColorGroup toColorGroup() const;
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

    QFont &asFont();
    QPixmap &asPixmap();
    QImage &asImage();
    QBrush &asBrush();
    QColor &asColor();
#ifndef QT_NO_PALETTE
    QPalette &asPalette();
#ifndef QT_NO_COMPAT
    QColorGroup &asColorGroup();
#endif // QT_NO_COMPAT
#endif // QT_NO_PALETTE
    QIconSet &asIconSet();
    QPointArray &asPointArray();
    QBitmap &asBitmap();
    QRegion &asRegion();
#ifndef QT_NO_CURSOR
    QCursor &asCursor();
#endif // QT_NO_CURSOR
#ifndef QT_NO_ACCEL
    QKeySequence &asKeySequence();
#endif // QT_NO_ACCEL
    QPen &asPen();
    QSizePolicy &asSizePolicy();

private:
    friend class QApplicationPrivate;
};

// Copied from qkernelvariant.h
inline QVariant::QVariant(Type type, void *v) : QKernelVariant(type, v) { }
inline QVariant::QVariant(const QVariant &other) : QKernelVariant(other) { }
inline QVariant::QVariant(const QKernelVariant &other) : QKernelVariant(other) { }

inline QVariant::QVariant(int i) : QKernelVariant(i) {};
inline QVariant::QVariant(uint ui) : QKernelVariant(ui) {};
inline QVariant::QVariant(Q_LLONG ll) : QKernelVariant(ll) {};
inline QVariant::QVariant(Q_ULLONG ull) : QKernelVariant(ull) {};
inline QVariant::QVariant(bool b) : QKernelVariant(b) {};
inline QVariant::QVariant(double d) : QKernelVariant(d) {};

inline QVariant::QVariant(const char *str) : QKernelVariant(str) {};
inline QVariant::QVariant(const QByteArray &bytearray) : QKernelVariant(bytearray) {};
inline QVariant::QVariant(const QBitArray &bitarray) : QKernelVariant(bitarray) {};
inline QVariant::QVariant(const QString &string) : QKernelVariant(string) {};
#ifndef QT_NO_STRINGLIST
inline QVariant::QVariant(const QStringList &stringlist) : QKernelVariant(stringlist) {};
#endif

inline QVariant::QVariant(const QDate &date) : QKernelVariant(date) {};
inline QVariant::QVariant(const QTime &time) : QKernelVariant(time) {};
inline QVariant::QVariant(const QDateTime &datetime) : QKernelVariant(datetime) {};
#ifndef QT_NO_TEMPLATE_VARIANT
inline QVariant::QVariant(const QList<QKernelVariant> &list) : QKernelVariant(list) {};
inline QVariant::QVariant(const QMap<QString,QKernelVariant> &map) : QKernelVariant(map) {};
#endif
inline QVariant::QVariant(const QSize &size) : QKernelVariant(size) {}
inline QVariant::QVariant(const QRect &rect) : QKernelVariant(rect) {}
inline QVariant::QVariant(const QPoint &pt) : QKernelVariant(pt) {}

inline QVariant::QVariant()
    : QKernelVariant()
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
#ifndef QT_NO_COMPAT
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
#ifndef QT_NO_COMPAT
inline QColorGroup& QVariant::asColorGroup()
{ return *static_cast<QColorGroup *>(castOrDetach(ColorGroup)); }
#endif // QT_NO_COMPAT
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

#ifndef QT_NO_COMPAT
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

#endif // QT_NO_VARIANT
#endif // QVARIANT_H

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

#ifndef QVARIANT_H
#define QVARIANT_H

#ifndef QT_NO_VARIANT

#include "QtCore/qcorevariant.h"
#include "QtCore/qmap.h"

class QFont;
class QPixmap;
class QBrush;
class QColor;
class QPalette;
#ifdef QT3_SUPPORT
class QColorGroup;
#endif
class QIcon;
class QTextFormat;
class QTextLength;
class QDataStream;
class QPolygon;
class QRegion;
class QBitmap;
class QCursor;
class QStringList;
class QSizePolicy;
class QKeySequence;
class QPen;
class QVariant;
class QApplicationPrivate;
class QImage;
class QPixmap;

class Q_GUI_EXPORT QVariant : public QCoreVariant
{
 public:
    inline QVariant();

    QVariant(const QFont &font);
    QVariant(const QPixmap &pixmap);
    QVariant(const QImage &image);
    QVariant(const QBrush &brush);
    QVariant(const QColor &color);
#ifndef QT_NO_PALETTE
    QVariant(const QPalette &palette);
#ifdef QT3_SUPPORT
    QT3_SUPPORT_CONSTRUCTOR QVariant(const QColorGroup &cg);
#endif // QT3_SUPPORT
#endif // QT_NO_PALETTE
#ifndef QT_NO_ICON
    QVariant(const QIcon &icon);
#endif
    QVariant(const QTextLength &length);
    QVariant(const QTextFormat &format);
    QVariant(const QPolygon &pointarray);
    QVariant(const QRegion &region);
    QVariant(const QBitmap &bitmap);
#ifndef QT_NO_CURSOR
    QVariant(const QCursor &cursor);
#endif
#ifndef QT_NO_ACCEL
    QVariant(const QKeySequence &keysequence);
#endif
    QVariant(const QPen &pen);
    QVariant(const QSizePolicy &sp);

    // Copied from qcorevariant.h
    QVariant(int typeOrUserType, const void *v);
    inline QVariant(Type type);
    inline QVariant(const QCoreVariant &other);
    inline QVariant(const QVariant &other);

    inline QVariant(int i);
    inline QVariant(uint ui);
    inline QVariant(qint64 ll);
    inline QVariant(quint64 ull);
    inline QVariant(double d);
    inline QVariant(bool b);
#ifdef QT3_SUPPORT
    inline QT3_SUPPORT_CONSTRUCTOR QVariant(bool b, int);
#endif

    inline QVariant(const char *str);
    inline QVariant(const QByteArray &bytearray);
    inline QVariant(const QBitArray &bitarray);
    inline QVariant(const QString &string);
    inline QVariant(const QChar &chr);
    inline QVariant(const QLatin1String &string);
    inline QVariant(const QStringList &stringlist);
    inline QVariant(const QPoint &point);
    inline QVariant(const QSize &size);
    inline QVariant(const QRect &rect);
    inline QVariant(const QUrl &url);

    inline QVariant(const QDate &date);
    inline QVariant(const QTime &time);
    inline QVariant(const QDateTime &datetime);
#ifndef QT_NO_TEMPLATE_VARIANT
    inline QVariant(const QList<QCoreVariant> &list);
    inline QVariant(const QMap<QString,QCoreVariant> &map);
    QVariant(const QList<QVariant> &list);
    QVariant(const QMap<QString, QVariant> &map);
#endif

    QFont toFont() const;
    QPixmap toPixmap() const;
    const QImage toImage() const;
    QBrush toBrush() const;
    QColor toColor() const;
    QPalette toPalette() const;
#ifdef QT3_SUPPORT
    QT3_SUPPORT QColorGroup toColorGroup() const;
#endif
    QIcon toIcon() const;
    QTextFormat toTextFormat() const;
    QTextLength toTextLength() const;
    const QPolygon toPolygon() const;
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

#ifdef QT3_SUPPORT
    QIcon toIconSet() const;
    inline QT3_SUPPORT QFont& asFont() { return *reinterpret_cast<QFont *>(castOrDetach(Font)); }
    inline QT3_SUPPORT QImage& asImage() { return *reinterpret_cast<QImage *>(castOrDetach(Image)); }
    inline QT3_SUPPORT QBrush& asBrush() { return *reinterpret_cast<QBrush *>(castOrDetach(Brush)); }
    inline QT3_SUPPORT QColor& asColor() { return *reinterpret_cast<QColor *>(castOrDetach(Color)); }
#ifndef QT_NO_PALETTE
    inline QT3_SUPPORT QPalette& asPalette() { return *reinterpret_cast<QPalette *>(castOrDetach(Palette)); }
    inline QT3_SUPPORT QColorGroup& asColorGroup() { return *reinterpret_cast<QColorGroup *>(castOrDetach(ColorGroup)); }
#endif // QT_NO_PALETTE
#ifndef QT_NO_ICON
    inline QT3_SUPPORT QIcon &asIconSet() { return *reinterpret_cast<QIcon *>(castOrDetach(IconSet)); }
#endif
    inline QT3_SUPPORT QPolygon& asPointArray() { return *reinterpret_cast<QPolygon *>(castOrDetach(Polygon)); }
    inline QT3_SUPPORT QBitmap& asBitmap() { return *reinterpret_cast<QBitmap *>(castOrDetach(Bitmap)); }
    inline QT3_SUPPORT QRegion& asRegion() { return *reinterpret_cast<QRegion *>(castOrDetach(Region)); }
#ifndef QT_NO_CURSOR
    inline QT3_SUPPORT QCursor& asCursor() { return *reinterpret_cast<QCursor *>(castOrDetach(Cursor)); }
#endif
#ifndef QT_NO_ACCEL
    inline QT3_SUPPORT QKeySequence& asKeySequence() { return *reinterpret_cast<QKeySequence *>(castOrDetach(KeySequence)); }
#endif
    inline QT3_SUPPORT QPen& asPen() { return *reinterpret_cast<QPen *>(castOrDetach(Pen)); }
    inline QT3_SUPPORT QSizePolicy& asSizePolicy()
        { return *reinterpret_cast<QSizePolicy *>(castOrDetach(SizePolicy)); }
    inline QT3_SUPPORT QPixmap& asPixmap() { return *reinterpret_cast<QPixmap *>(castOrDetach(Pixmap)); }
#endif //QT3_SUPPORT

protected:
    void create(int type, const void *copy);
};

typedef QList<QVariant> QVariantList;
typedef QMap<QString, QVariant> QVariantMap;

inline QVariant::QVariant() : QCoreVariant() { }

inline QVariant::QVariant(Type type): QCoreVariant(type) { }
inline QVariant::QVariant(const QVariant &other) : QCoreVariant(other) { }
inline QVariant::QVariant(const QCoreVariant &other) : QCoreVariant(other) { }

inline QVariant::QVariant(int i) : QCoreVariant(i) {};
inline QVariant::QVariant(uint ui) : QCoreVariant(ui) {};
inline QVariant::QVariant(qint64 ll) : QCoreVariant(ll) {};
inline QVariant::QVariant(quint64 ull) : QCoreVariant(ull) {};
inline QVariant::QVariant(bool b) : QCoreVariant(b) {};
inline QVariant::QVariant(double d) : QCoreVariant(d) {};
#ifdef QT3_SUPPORT
inline QVariant::QVariant(bool b, int) : QCoreVariant(b) {};
#endif

inline QVariant::QVariant(const char *str) : QCoreVariant(str) {}
inline QVariant::QVariant(const QByteArray &bytearray) : QCoreVariant(bytearray) {}
inline QVariant::QVariant(const QBitArray &bitarray) : QCoreVariant(bitarray) {}
inline QVariant::QVariant(const QString &string) : QCoreVariant(string) {}
inline QVariant::QVariant(const QChar &chr) : QCoreVariant(chr) {}
inline QVariant::QVariant(const QLatin1String &string) : QCoreVariant(string) {}
inline QVariant::QVariant(const QStringList &stringlist) : QCoreVariant(stringlist) {}
inline QVariant::QVariant(const QSize &size): QCoreVariant(size) {}
inline QVariant::QVariant(const QPoint &point): QCoreVariant(point) {}
inline QVariant::QVariant(const QRect &rect): QCoreVariant(rect) {}
inline QVariant::QVariant(const QUrl &url): QCoreVariant(url) {}

inline QVariant::QVariant(const QDate &date) : QCoreVariant(date) {}
inline QVariant::QVariant(const QTime &time) : QCoreVariant(time) {}
inline QVariant::QVariant(const QDateTime &datetime) : QCoreVariant(datetime) {}
#ifndef QT_NO_TEMPLATE_VARIANT
inline QVariant::QVariant(const QList<QCoreVariant> &list) : QCoreVariant(list) {}
inline QVariant::QVariant(const QMap<QString, QCoreVariant> &map) : QCoreVariant(map) {}
inline QVariant::QVariant(const QList<QVariant> &list)
    : QCoreVariant(*reinterpret_cast<const QList<QCoreVariant>*>(&list)) {}
inline QVariant::QVariant(const QMap<QString, QVariant> &map)
    : QCoreVariant(*reinterpret_cast<const QMap<QString, QCoreVariant>*>(&map)) {}
#endif

Q_DECLARE_SHARED(QVariant);
Q_DECLARE_TYPEINFO(QVariant, Q_MOVABLE_TYPE);

#ifndef QT_NO_DEBUG_OUTPUT
Q_GUI_EXPORT QDebug operator<<(QDebug, const QVariant &);
#endif

#endif // QT_NO_VARIANT

#endif // QVARIANT_H

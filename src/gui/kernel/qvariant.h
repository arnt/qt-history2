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

#include "qcorevariant.h"
#include "qmap.h"

class QFont;
class QPixmap;
class QBrush;
class QColor;
class QPalette;
#ifdef QT_COMPAT
class QColorGroup;
#endif
class QIcon;
class QTextLength;
class QDataStream;
class QPolygon;
class QRegion;
class QBitmap;
class QCursor;
class QObject;
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
#ifdef QT_COMPAT
    QT_COMPAT_CONSTRUCTOR QVariant(const QColorGroup &cg);
#endif // QT_COMPAT
#endif // QT_NO_PALETTE
#ifndef QT_NO_ICON
    QVariant(const QIcon &icon);
#endif
    QVariant(const QTextLength &length);
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
    inline QVariant(Q_LONGLONG ll);
    inline QVariant(Q_ULONGLONG ull);
    inline QVariant(double d);
    inline QVariant(bool b);
#ifdef QT_COMPAT
    inline QT_COMPAT_CONSTRUCTOR QVariant(bool b, int);
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
    inline QVariant(QObject *object);
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
#ifdef QT_COMPAT
    QT_COMPAT QColorGroup toColorGroup() const;
#endif
    QIcon toIcon() const;
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

#ifdef QT_COMPAT
    QIcon toIconSet() const;
    inline QT_COMPAT QFont& asFont() { return *reinterpret_cast<QFont *>(castOrDetach(Font)); }
    inline QT_COMPAT QImage& asImage() { return *reinterpret_cast<QImage *>(castOrDetach(Image)); }
    inline QT_COMPAT QBrush& asBrush() { return *reinterpret_cast<QBrush *>(castOrDetach(Brush)); }
    inline QT_COMPAT QColor& asColor() { return *reinterpret_cast<QColor *>(castOrDetach(Color)); }
#ifndef QT_NO_PALETTE
    inline QT_COMPAT QPalette& asPalette() { return *reinterpret_cast<QPalette *>(castOrDetach(Palette)); }
    inline QT_COMPAT QColorGroup& asColorGroup() { return *reinterpret_cast<QColorGroup *>(castOrDetach(ColorGroup)); }
#endif // QT_NO_PALETTE
#ifndef QT_NO_ICON
    inline QT_COMPAT QIcon &asIconSet() { return *reinterpret_cast<QIcon *>(castOrDetach(IconSet)); }
#endif
    inline QT_COMPAT QPolygon& asPointArray() { return *reinterpret_cast<QPolygon *>(castOrDetach(PointArray)); }
    inline QT_COMPAT QBitmap& asBitmap() { return *reinterpret_cast<QBitmap *>(castOrDetach(Bitmap)); }
    inline QT_COMPAT QRegion& asRegion() { return *reinterpret_cast<QRegion *>(castOrDetach(Region)); }
#ifndef QT_NO_CURSOR
    inline QT_COMPAT QCursor& asCursor() { return *reinterpret_cast<QCursor *>(castOrDetach(Cursor)); }
#endif
#ifndef QT_NO_ACCEL
    inline QT_COMPAT QKeySequence& asKeySequence() { return *reinterpret_cast<QKeySequence *>(castOrDetach(KeySequence)); }
#endif
    inline QT_COMPAT QPen& asPen() { return *reinterpret_cast<QPen *>(castOrDetach(Pen)); }
    inline QT_COMPAT QSizePolicy& asSizePolicy()
        { return *reinterpret_cast<QSizePolicy *>(castOrDetach(SizePolicy)); }
    inline QT_COMPAT QPixmap& asPixmap() { return *reinterpret_cast<QPixmap *>(castOrDetach(Pixmap)); }
#endif //QT_COMPAT

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
inline QVariant::QVariant(Q_LONGLONG ll) : QCoreVariant(ll) {};
inline QVariant::QVariant(Q_ULONGLONG ull) : QCoreVariant(ull) {};
inline QVariant::QVariant(bool b) : QCoreVariant(b) {};
inline QVariant::QVariant(double d) : QCoreVariant(d) {};
#ifdef QT_COMPAT
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
inline QVariant::QVariant(QObject *object) : QCoreVariant(object) {}
#ifndef QT_NO_TEMPLATE_VARIANT
inline QVariant::QVariant(const QList<QCoreVariant> &list) : QCoreVariant(list) {}
inline QVariant::QVariant(const QMap<QString, QCoreVariant> &map) : QCoreVariant(map) {}
inline QVariant::QVariant(const QList<QVariant> &list)
    : QCoreVariant(*reinterpret_cast<const QList<QCoreVariant>*>(&list)) {}
inline QVariant::QVariant(const QMap<QString, QVariant> &map)
    : QCoreVariant(*reinterpret_cast<const QMap<QString, QCoreVariant>*>(&map)) {}
#endif

#if defined Q_CC_MSVC && _MSC_VER < 1300

template<> QFont QVariant_to_helper<QFont>(const QCoreVariant &v, const QFont*);
template<> QPixmap QVariant_to_helper<QPixmap>(const QCoreVariant &v, const QPixmap*);
template<> QImage QVariant_to_helper<QImage>(const QCoreVariant &v, const QImage*);
template<> QBrush QVariant_to_helper<QBrush>(const QCoreVariant &v, const QBrush*);
template<> QColor QVariant_to_helper<QColor>(const QCoreVariant &v, const QColor*);
template<> QPalette QVariant_to_helper<QPalette>(const QCoreVariant &v, const QPalette*);
template<> QIcon QVariant_to_helper<QIcon>(const QCoreVariant &v, const QIcon*);
template<> QTextLength QVariant_to_helper<QTextLength>(const QCoreVariant &v, const QTextLength*);
template<> QPolygon QVariant_to_helper<QPolygon>(const QCoreVariant &v, const QPolygon*);
template<> QBitmap QVariant_to_helper<QBitmap>(const QCoreVariant &v, const QBitmap*);
template<> QRegion QVariant_to_helper<QRegion>(const QCoreVariant &v, const QRegion*);
#ifndef QT_NO_CURSOR
template<> QCursor QVariant_to_helper<QCursor>(const QCoreVariant &v, const QCursor*);
#endif
#ifndef QT_NO_ACCEL
template<> QKeySequence QVariant_to_helper<QKeySequence>(const QCoreVariant &v, const QKeySequence*);
#endif
template<> QPen QVariant_to_helper<QPen>(const QCoreVariant &v, const QPen*);
template<> QSizePolicy QVariant_to_helper<QSizePolicy>(const QCoreVariant &v, const QSizePolicy*);

#else

template<> QFont QVariant_to<QFont>(const QCoreVariant &v);
template<> QPixmap QVariant_to<QPixmap>(const QCoreVariant &v);
template<> QImage QVariant_to<QImage>(const QCoreVariant &v);
template<> QBrush QVariant_to<QBrush>(const QCoreVariant &v);
template<> QColor QVariant_to<QColor>(const QCoreVariant &v);
template<> QPalette QVariant_to<QPalette>(const QCoreVariant &v);
template<> QIcon QVariant_to<QIcon>(const QCoreVariant &v);
template<> QTextLength QVariant_to<QTextLength>(const QCoreVariant &v);
template<> QPolygon QVariant_to<QPolygon>(const QCoreVariant &v);
template<> QBitmap QVariant_to<QBitmap>(const QCoreVariant &v);
template<> QRegion QVariant_to<QRegion>(const QCoreVariant &v);
#ifndef QT_NO_CURSOR
template<> QCursor QVariant_to<QCursor>(const QCoreVariant &v);
#endif
#ifndef QT_NO_ACCEL
template<> QKeySequence QVariant_to<QKeySequence>(const QCoreVariant &v);
#endif
template<> QPen QVariant_to<QPen>(const QCoreVariant &v);
template<> QSizePolicy QVariant_to<QSizePolicy>(const QCoreVariant &v);

#endif

Q_DECLARE_SHARED(QVariant);
Q_DECLARE_TYPEINFO(QVariant, Q_MOVABLE_TYPE);

#ifndef QT_NO_DEBUG_OUTPUT
Q_GUI_EXPORT QDebug operator<<(QDebug, const QVariant &);
#endif

#endif // QT_NO_VARIANT
#endif // QVARIANT_H

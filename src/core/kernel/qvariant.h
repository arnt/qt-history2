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

#include "QtCore/qatomic.h"
#include "QtCore/qbytearray.h"
#include "QtCore/qlist.h"
#include "QtCore/qmetatype.h"

#ifndef QT_NO_VARIANT
class QBitArray;
class QDataStream;
class QDate;
class QDateTime;
class QLineF;
class QLocale;
class QString;
class QStringList;
class QTime;
class QPoint;
class QPointF;
class QSize;
class QRect;
class QRectF;
class QTextFormat;
class QTextLength;
class QUrl;
class QVariant;
class QVariantComparisonHelper;

template <class Key, class Type> class QMap;


#ifndef Q_NO_MEMBER_TEMPLATES
template <typename T>
inline QVariant qVariantFromValue(const T &);

template <typename T>
inline void qVariantSetValue(QVariant &, const T &);

template<typename T>
inline T qVariantValue(const QVariant &);

template<typename T>
inline bool qVariantCanConvert(const QVariant &);
#endif

class Q_CORE_EXPORT QVariant
{
 public:
    enum Type {
        Invalid = 0,
        Map = 1,
        List = 2,
        String = 3,
        StringList = 4,
        Font = 5,
        Pixmap = 6,
        Brush = 7,
        Rect = 8,
        Size = 9,
        Color = 10,
        Palette = 11,
        Icon = 13,
        Point = 14,
        Image = 15,
        Int = 16,
        UInt = 17,
        Bool = 18,
        Double = 19,
        Polygon = 21,
        Region = 22,
        Bitmap = 23,
        Cursor = 24,
        SizePolicy = 25,
        Date = 26,
        Time = 27,
        DateTime = 28,
        ByteArray = 29,
        BitArray = 30,
        KeySequence = 31,
        Pen = 32,
        LongLong = 33,
        ULongLong = 34,
        Char = 35,
        Url = 36,
        TextLength = 37,
        TextFormat = 38,
        Locale = 39,
        LineF = 40,
        RectF = 41,
        PointF = 42,
        UserType = 127,
        LastType = 0xffffffff // need this so that gcc >= 3.4 allocates 32 bits for Type
#ifdef QT3_SUPPORT
        , ColorGroup = 12,
        IconSet = Icon,
        CString = ByteArray,
        PointArray = Polygon
#endif
    };

    inline QVariant();
    ~QVariant();
    QVariant(Type type);
    QVariant(int typeOrUserType, const void *copy);
    QVariant(const QVariant &other);

#ifndef QT_NO_DATASTREAM
    QVariant(QDataStream &s);
#endif

    QVariant(int i);
    QVariant(uint ui);
    QVariant(qlonglong ll);
    QVariant(qulonglong ull);
    QVariant(bool b);
    QVariant(double d);
    QVariant(const char *str);

    QVariant(const QByteArray &bytearray);
    QVariant(const QBitArray &bitarray);
    QVariant(const QString &string);
    QVariant(const QLatin1String &string);
    QVariant(const QStringList &stringlist);
    QVariant(const QChar &qchar);
    QVariant(const QDate &date);
    QVariant(const QTime &time);
    QVariant(const QDateTime &datetime);
#ifndef QT_NO_TEMPLATE_VARIANT
    QVariant(const QList<QVariant> &list);
    QVariant(const QMap<QString,QVariant> &map);
#endif
#ifndef QT_NO_GEOM_VARIANT
    QVariant(const QSize &size);
    QVariant(const QRect &rect);
    QVariant(const QPoint &pt);
    QVariant(const QPointF &pt);
    QVariant(const QLineF &line);
    QVariant(const QRectF &rect);
#endif
    QVariant(const QUrl &url);
    QVariant(const QLocale &locale);

    QVariant& operator=(const QVariant &other);

    Type type() const;
    int userType() const;
    const char *typeName() const;

    bool canConvert(Type t) const;
    bool convert(Type t);

#ifdef QT3_SUPPORT
    inline QT3_SUPPORT bool canCast(Type t) const
    { return canConvert(t); }
    inline QT3_SUPPORT bool cast(Type t)
    { return convert(t); }
#endif

    inline bool isValid() const;
    bool isNull() const;

    void clear();

    void detach();
    inline bool isDetached() const;

    int toInt(bool *ok = 0) const;
    uint toUInt(bool *ok = 0) const;
    qlonglong toLongLong(bool *ok = 0) const;
    qulonglong toULongLong(bool *ok = 0) const;
    bool toBool() const;
    double toDouble(bool *ok = 0) const;
    QByteArray toByteArray() const;
    QBitArray toBitArray() const;
    QString toString() const;
    QStringList toStringList() const;
    QChar toChar() const;
    QDate toDate() const;
    QTime toTime() const;
    QDateTime toDateTime() const;
#ifndef QT_NO_TEMPLATE_VARIANT
    QList<QVariant> toList() const;
    QMap<QString,QVariant> toMap() const;
#endif

#ifndef QT_NO_GEOM_VARIANT
    QPoint toPoint() const;
    QPointF toPointF() const;
    QRect toRect() const;
    QSize toSize() const;
    QLineF toLineF() const;
    QRectF toRectF() const;
#endif
    QUrl toUrl() const;
    QLocale toLocale() const;

#ifdef QT3_SUPPORT
    inline QT3_SUPPORT int &asInt();
    inline QT3_SUPPORT uint &asUInt();
    inline QT3_SUPPORT qlonglong &asLongLong();
    inline QT3_SUPPORT qulonglong &asULongLong();
    inline QT3_SUPPORT bool &asBool();
    inline QT3_SUPPORT double &asDouble();
    inline QT3_SUPPORT QByteArray &asByteArray();
    inline QT3_SUPPORT QBitArray &asBitArray();
    inline QT3_SUPPORT QString &asString();
    inline QT3_SUPPORT QStringList &asStringList();
    inline QT3_SUPPORT QDate &asDate();
    inline QT3_SUPPORT QTime &asTime();
    inline QT3_SUPPORT QDateTime &asDateTime();
#ifndef QT_NO_TEMPLATE_VARIANT
    inline QT3_SUPPORT QList<QVariant> &asList();
    inline QT3_SUPPORT QMap<QString,QVariant> &asMap();
#endif
    inline QT3_SUPPORT QPoint &asPoint();
    inline QT3_SUPPORT QRect &asRect();
    inline QT3_SUPPORT QSize &asSize();
#endif //QT3_SUPPORT

#ifndef QT_NO_DATASTREAM
    void load(QDataStream &ds);
    void save(QDataStream &ds) const;
#endif
    static const char *typeToName(Type type);
    static Type nameToType(const char *name);

#ifdef QT3_SUPPORT
    inline QT3_SUPPORT_CONSTRUCTOR QVariant(bool val, int) { create(Bool, &val); }
    inline QT3_SUPPORT const QByteArray toCString() const { return toByteArray(); }
    inline QT3_SUPPORT QByteArray &asCString() { return *reinterpret_cast<QByteArray *>(castOrDetach(ByteArray)); }
#endif

    void *data();
    const void *constData() const;
    inline const void *data() const { return constData(); }

#ifndef Q_NO_MEMBER_TEMPLATES
    template<typename T>
    inline void setValue(const T &value)
    { return qVariantSetValue(*this, value); }

    template<typename T>
    inline T value() const
    { return qVariantValue<T>(*this); }

    template<typename T>
    static inline QVariant fromValue(const T &value)
    { return qVariantFromValue(value); }

    template<typename T>
    bool canConvert() const
    { return qVariantCanConvert<T>(); }
#endif

 public:
#ifndef qdoc
    struct PrivateShared
    {
        inline PrivateShared() : ref(1) { }
        inline PrivateShared(void *v) : ref(1) { ptr = v; }
        void *ptr;
        QAtomic ref;
    };
    struct Private
    {
        inline Private(): type(Invalid), is_shared(false), is_null(true) { data.ptr = 0; }
        union Data
        {
            int i;
            uint u;
            bool b;
            double d;
            qlonglong ll;
            qulonglong ull;
            void *ptr;
            PrivateShared *shared;
        } data;
        uint type : 30;
        uint is_shared : 1;
        uint is_null : 1;
    };
 public:
    typedef void (*f_construct)(Private *, const void *);
    typedef void (*f_clear)(Private *);
    typedef bool (*f_null)(const Private *);
#ifndef QT_NO_DATASTREAM
    typedef void (*f_load)(Private *, QDataStream &);
    typedef void (*f_save)(const Private *, QDataStream &);
#endif
    typedef bool (*f_compare)(const Private *, const Private *);
    typedef bool (*f_convert)(const QVariant::Private *d, Type t, void *, bool *);
    typedef bool (*f_canConvert)(const QVariant::Private *d, Type t);
    typedef void (*f_debugStream)(QDebug, const QVariant &);
    struct Handler {
        f_construct construct;
        f_clear clear;
        f_null isNull;
#ifndef QT_NO_DATASTREAM
        f_load load;
        f_save save;
#endif
        f_compare compare;
        f_convert convert;
        f_canConvert canConvert;
        f_debugStream debugStream;
    };
#endif

    inline bool operator==(const QVariant &v) const
    { return cmp(v); }
    inline bool operator!=(const QVariant &v) const
    { return !cmp(v); }

protected:
    friend inline bool qvariant_cast_helper(const QVariant &, QVariant::Type, void *);
    friend bool qRegisterGuiVariant();
    friend inline bool operator==(const QVariant &,
                                  const QVariantComparisonHelper &);
#ifndef QT_NO_DEBUG_STREAM
    friend Q_CORE_EXPORT QDebug operator<<(QDebug, const QVariant &);
#endif
    Private d;

#ifndef QT_MOC
    static const Handler *handler;
#endif

    void create(int type, const void *copy);
#ifdef QT3_SUPPORT
    void *castOrDetach(Type t);
#endif
    bool cmp(const QVariant &other) const;
};

#ifndef QT_MOC
typedef QList<QVariant> QVariantList;
typedef QMap<QString, QVariant> QVariantMap;

inline bool qvariant_cast_helper(const QVariant &v, QVariant::Type tp, void *ptr)
{ return QVariant::handler->convert(&v.d, tp, ptr, 0); }

template <typename T>
inline int qt_variant_metatype_id(T *)
{ return QMetaTypeId<T>::qt_metatype_id(); }

template<>
inline int qt_variant_metatype_id(QVariantMap *) { return QVariant::Map; }
template<>
inline int qt_variant_metatype_id(QVariantList *) { return QVariant::List; }
template<>
inline int qt_variant_metatype_id(QStringList *) { return QVariant::StringList; }
class QFont;
template<>
inline int qt_variant_metatype_id(QFont *) { return QVariant::Font; }
class QPixmap;
template<>
inline int qt_variant_metatype_id(QPixmap *) { return QVariant::Pixmap; }
class QBrush;
template<>
inline int qt_variant_metatype_id(QBrush *) { return QVariant::Brush; }
template<>
inline int qt_variant_metatype_id(QRect *) { return QVariant::Rect; }
template<>
inline int qt_variant_metatype_id(QSize *) { return QVariant::Size; }
class QColor;
template<>
inline int qt_variant_metatype_id(QColor *) { return QVariant::Color; }
class QPalette;
template<>
inline int qt_variant_metatype_id(QPalette *) { return QVariant::Palette; }
class QIcon;
template<>
inline int qt_variant_metatype_id(QIcon *) { return QVariant::Icon; }
class QPoint;
template<>
inline int qt_variant_metatype_id(QPoint *) { return QVariant::Point; }
class QPointF;
template<>
inline int qt_variant_metatype_id(QPointF *) { return QVariant::PointF; }
class QImage;
template<>
inline int qt_variant_metatype_id(QImage*) { return QVariant::Image; }
class QPolygon;
template<>
inline int qt_variant_metatype_id(QPolygon *) { return QVariant::Polygon; }
class QRegion;
template<>
inline int qt_variant_metatype_id(QRegion *) { return QVariant::Region; }
class QBitmap;
template<>
inline int qt_variant_metatype_id(QBitmap *) { return QVariant::Bitmap; }
class QCursor;
template<>
inline int qt_variant_metatype_id(QCursor *) { return QVariant::Cursor; }
class QSizePolicy;
template<>
inline int qt_variant_metatype_id(QSizePolicy *) { return QVariant::SizePolicy; }
template<>
inline int qt_variant_metatype_id(QDate *) { return QVariant::Date; }
template<>
inline int qt_variant_metatype_id(QTime *) { return QVariant::Time; }
template<>
inline int qt_variant_metatype_id(QDateTime *) { return QVariant::DateTime; }
template<>
inline int qt_variant_metatype_id(QBitArray *) { return QVariant::BitArray; }
class QKeySequence;
template<>
inline int qt_variant_metatype_id(QKeySequence *) { return QVariant::KeySequence; }
class QPen;
template<>
inline int qt_variant_metatype_id(QPen *) { return QVariant::Pen; }
template<>
inline int qt_variant_metatype_id(qlonglong *) { return QVariant::LongLong; }
template<>
inline int qt_variant_metatype_id(qulonglong *) { return QVariant::ULongLong; }
template<>
inline int qt_variant_metatype_id(QUrl *) { return QVariant::Url; }
class QTextLength;
template<>
inline int qt_variant_metatype_id(QTextLength *) { return QVariant::TextLength; }
class QTextFormat;
template<>
inline int qt_variant_metatype_id(QTextFormat *) { return QVariant::TextFormat; }
template<>
inline int qt_variant_metatype_id(QLocale *) { return QVariant::Locale; }
template<>
inline int qt_variant_metatype_id(QLineF *) { return QVariant::LineF; }
template<>
inline int qt_variant_metatype_id(QRectF *) { return QVariant::RectF; }
#ifdef QT3_SUPPORT
class QColorGroup;
template<>
inline int qt_variant_metatype_id(QColorGroup *) { return QVariant::ColorGroup; }
#endif

template <typename T>
inline QVariant qVariantFromValue(const T &t)
{
    return QVariant(qt_variant_metatype_id<T>(static_cast<T *>(0)), &t);
}

template <>
inline QVariant qVariantFromValue(const QVariant &t) { return t; }

template <typename T>
inline void qVariantSetValue(QVariant &v, const T &t)
{
    v = QVariant(qt_variant_metatype_id<T>(static_cast<T *>(0)), &t);
}
#endif

inline QVariant::QVariant() {}
inline bool QVariant::isValid() const { return d.type != Invalid; }

#ifdef QT3_SUPPORT
inline int &QVariant::asInt()
{ return *reinterpret_cast<int *>(castOrDetach(Int)); }
inline uint &QVariant::asUInt()
{ return *reinterpret_cast<uint *>(castOrDetach(UInt)); }
inline qlonglong &QVariant::asLongLong()
{ return *reinterpret_cast<qlonglong *>(castOrDetach(LongLong)); }
inline qulonglong &QVariant::asULongLong()
{ return *reinterpret_cast<qulonglong *>(castOrDetach(ULongLong)); }
inline bool &QVariant::asBool()
{ return *reinterpret_cast<bool *>(castOrDetach(Bool)); }
inline double &QVariant::asDouble()
{ return *reinterpret_cast<double *>(castOrDetach(Double)); }
inline QByteArray& QVariant::asByteArray()
{ return *reinterpret_cast<QByteArray *>(castOrDetach(ByteArray)); }
inline QBitArray& QVariant::asBitArray()
{ return *reinterpret_cast<QBitArray *>(castOrDetach(BitArray)); }
inline QString& QVariant::asString()
{ return *reinterpret_cast<QString *>(castOrDetach(String)); }
inline QStringList& QVariant::asStringList()
{ return *reinterpret_cast<QStringList *>(castOrDetach(StringList)); }
inline QDate& QVariant::asDate()
{ return *reinterpret_cast<QDate *>(castOrDetach(Date)); }
inline QTime& QVariant::asTime()
{ return *reinterpret_cast<QTime *>(castOrDetach(Time)); }
inline QDateTime& QVariant::asDateTime()
{ return *reinterpret_cast<QDateTime *>(castOrDetach(DateTime)); }
#ifndef QT_NO_TEMPLATE_VARIANT
inline QList<QVariant>& QVariant::asList()
{ return *reinterpret_cast<QList<QVariant> *>(castOrDetach(List)); }
inline QMap<QString, QVariant>& QVariant::asMap()
{ return *reinterpret_cast<QMap<QString, QVariant> *>(castOrDetach(Map)); }
#endif
inline QPoint &QVariant::asPoint()
{ return *reinterpret_cast<QPoint *>(castOrDetach(Point)); }
inline QRect &QVariant::asRect()
{ return *reinterpret_cast<QRect *>(castOrDetach(Rect)); }
inline QSize &QVariant::asSize()
{ return *reinterpret_cast<QSize *>(castOrDetach(Size)); }
#endif //QT3_SUPPORT

#ifndef QT_NO_DATASTREAM
Q_CORE_EXPORT QDataStream& operator>> (QDataStream& s, QVariant& p);
Q_CORE_EXPORT QDataStream& operator<< (QDataStream& s, const QVariant& p);
Q_CORE_EXPORT QDataStream& operator>> (QDataStream& s, QVariant::Type& p);
Q_CORE_EXPORT QDataStream& operator<< (QDataStream& s, const QVariant::Type p);
#endif

inline bool QVariant::isDetached() const
{ return !d.is_shared || d.data.shared->ref == 1; }


#ifdef qdoc
    inline bool operator==(const QVariant &v1, const QVariant &v2);
    inline bool operator!=(const QVariant &v1, const QVariant &v2);
#else

/* Helper class to add one more level of indirection to prevent
   implicit casts.
*/
class QVariantComparisonHelper
{
public:
    inline QVariantComparisonHelper(const QVariant &var)
        : v(&var) {}
private:
    friend inline bool operator==(const QVariant &,
                                  const QVariantComparisonHelper &);
    const QVariant *v;
};

inline bool operator==(const QVariant &v1, const QVariantComparisonHelper &v2)
{
    return v1.cmp(*v2.v);
}

inline bool operator!=(const QVariant &v1, const QVariantComparisonHelper &v2)
{
    return !operator==(v1, v2);
}
#endif

#ifndef QT_MOC
#if defined Q_CC_MSVC && _MSC_VER < 1300

template<typename T> T qvariant_cast(const QVariant &v, T * = 0)
{
    const int vid = qt_variant_metatype_id<T>(static_cast<T *>(0));
    if (vid == v.userType())
        return *reinterpret_cast<const T *>(v.constData());
    if (vid < int(QVariant::UserType)) {
        T t;
        qvariant_cast_helper(v, QVariant::Type(vid), &t);
        return t;
    }
    return T();
}

template<typename T>
inline T qVariantValue(const QVariant &variant, T *t = 0)
{ return qvariant_cast<T>(variant, t); }

template<typename T>
inline bool qVariantCanConvert(const QVariant &variant, T *t = 0)
{
    return variant.canConvert(static_cast<QVariant::Type>(qt_variant_metatype_id<T>(t)));
}
#else

template<typename T> T qvariant_cast(const QVariant &v)
{
    const int vid = qt_variant_metatype_id<T>(static_cast<T *>(0));
    if (vid == v.userType())
        return *reinterpret_cast<const T *>(v.constData());
    if (vid < int(QVariant::UserType)) {
        T t;
        qvariant_cast_helper(v, QVariant::Type(vid), &t);
        return t;
    }
    return T();
}

template<typename T>
inline T qVariantValue(const QVariant &variant)
{ return qvariant_cast<T>(variant); }

template<typename T>
inline bool qVariantCanConvert(const QVariant &variant)
{
    return variant.canConvert(static_cast<QVariant::Type>(
                qt_variant_metatype_id<T>(static_cast<T *>(0))));
}
#endif
#endif
Q_DECLARE_SHARED(QVariant);
Q_DECLARE_TYPEINFO(QVariant, Q_MOVABLE_TYPE);

#ifndef QT_NO_DEBUG_STREAM
Q_CORE_EXPORT QDebug operator<<(QDebug, const QVariant &);
Q_CORE_EXPORT QDebug operator<<(QDebug, const QVariant::Type);
#endif

#endif //QT_NO_VARIANT

#endif // QVARIANT_H

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

#ifndef QCOREVARIANT_H
#define QCOREVARIANT_H

#include "qatomic.h"
#include "qbytearray.h"
#include "qlist.h"
#include "qmetatype.h"

#ifndef QT_NO_VARIANT
class QBitArray;
class QDataStream;
class QDate;
class QDateTime;
class QString;
class QStringList;
class QTime;
class QPoint;
class QSize;
class QRect;
class QUrl;

template <class Key, class Type> class QMap;

class Q_CORE_EXPORT QCoreVariant
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
        PointArray = 21,
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
        UserType = 63,
        LastType = 0xffffffff // need this so that gcc >= 3.4 allocates 32 bits for Type
#ifdef QT_COMPAT
        , ColorGroup = 12,
        IconSet = Icon,
        CString = ByteArray
#endif
    };

    inline QCoreVariant();
    ~QCoreVariant();
    QCoreVariant(Type type);
    QCoreVariant(int typeOrUserType, const void *copy);
    QCoreVariant(const QCoreVariant &other);

#ifndef QT_NO_DATASTREAM
    QCoreVariant(QDataStream &s);
#endif

    QCoreVariant(int i);
    QCoreVariant(uint ui);
    QCoreVariant(Q_LONGLONG ll);
    QCoreVariant(Q_ULONGLONG ull);
    QCoreVariant(bool b);
    QCoreVariant(double d);

    QCoreVariant(const char *str);
    QCoreVariant(const QByteArray &bytearray);
    QCoreVariant(const QBitArray &bitarray);
    QCoreVariant(const QString &string);
    QCoreVariant(const QLatin1String &string);
    QCoreVariant(const QStringList &stringlist);
    QCoreVariant(const QChar &qchar);

    QCoreVariant(const QDate &date);
    QCoreVariant(const QTime &time);
    QCoreVariant(const QDateTime &datetime);
#ifndef QT_NO_TEMPLATE_VARIANT
    QCoreVariant(const QList<QCoreVariant> &list);
    QCoreVariant(const QMap<QString,QCoreVariant> &map);
#endif

    QCoreVariant(const QSize &size);
    QCoreVariant(const QRect &rect);
    QCoreVariant(const QPoint &pt);

    QCoreVariant(const QUrl &url);

    QCoreVariant& operator=(const QCoreVariant &other);

    bool operator==(const QCoreVariant &other) const;
    inline bool operator!=(const QCoreVariant &other) const
    { return !(other == *this); }

    Type type() const;
    int userType() const;
    const char *typeName() const;

    bool canCast(Type t) const;
    bool cast(Type t);

    inline bool isValid() const;
    bool isNull() const;

    void clear();

    void detach();
    inline bool isDetached() const;

    int toInt(bool *ok = 0) const;
    uint toUInt(bool *ok = 0) const;
    Q_LONGLONG toLongLong(bool *ok = 0) const;
    Q_ULONGLONG toULongLong(bool *ok = 0) const;
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
    QList<QCoreVariant> toList() const;
    QMap<QString,QCoreVariant> toMap() const;
#endif

    QPoint toPoint() const;
    QRect toRect() const;
    QSize toSize() const;

    QUrl toUrl() const;

#ifdef QT_COMPAT
    inline QT_COMPAT int &asInt();
    inline QT_COMPAT uint &asUInt();
    inline QT_COMPAT Q_LONGLONG &asLongLong();
    inline QT_COMPAT Q_ULONGLONG &asULongLong();
    inline QT_COMPAT bool &asBool();
    inline QT_COMPAT double &asDouble();
    inline QT_COMPAT QByteArray &asByteArray();
    inline QT_COMPAT QBitArray &asBitArray();
    inline QT_COMPAT QString &asString();
    inline QT_COMPAT QStringList &asStringList();
    inline QT_COMPAT QDate &asDate();
    inline QT_COMPAT QTime &asTime();
    inline QT_COMPAT QDateTime &asDateTime();
#ifndef QT_NO_TEMPLATE_VARIANT
    inline QT_COMPAT QList<QCoreVariant> &asList();
    inline QT_COMPAT QMap<QString,QCoreVariant> &asMap();
#endif
    inline QT_COMPAT QPoint &asPoint();
    inline QT_COMPAT QRect &asRect();
    inline QT_COMPAT QSize &asSize();
#endif //QT_COMPAT

#ifndef QT_NO_DATASTREAM
    void load(QDataStream &ds);
    void save(QDataStream &ds) const;
#endif
    static const char *typeToName(Type type);
    static Type nameToType(const char *name);

#ifdef QT_COMPAT
    QT_COMPAT_CONSTRUCTOR QCoreVariant(bool, int);
    inline QT_COMPAT const QByteArray toCString() const { return toByteArray(); }
    inline QT_COMPAT QByteArray &asCString() { return *static_cast<QByteArray *>(castOrDetach(ByteArray)); }
#endif

    void *data();
    const void *constData() const;
    inline const void *data() const { return constData(); }

 public:
#ifndef qdoc
    struct PrivateShared
    {
        inline PrivateShared() : ref(1) { }
        inline PrivateShared(void *v) : ref(1) { value.ptr = v; }
        QAtomic ref;
        union
        {
            Q_LONGLONG ll;
            Q_ULONGLONG ull;
            double d;
            void *ptr;
        } value;
    };
    struct Private
    {
        inline Private(): type(Invalid), is_shared(false), is_null(true) { data.ptr = 0; }
        uint type : 30;
        uint is_shared : 1;
        uint is_null : 1;
        union
        {
            int i;
            uint u;
            bool b;
            void *ptr;
            PrivateShared *shared;
        } data;
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
    typedef void (*f_cast)(const QCoreVariant::Private *d, Type t, void *, bool *);
    typedef bool (*f_canCast)(const QCoreVariant::Private *d, Type t);
    struct Handler {
        f_construct construct;
        f_clear clear;
        f_null isNull;
#ifndef QT_NO_DATASTREAM
        f_load load;
        f_save save;
#endif
        f_compare compare;
        f_cast cast;
        f_canCast canCast;
    };
#endif

protected:
    Private d;

    static const Handler *handler;

    void create(int type, const void *copy);
    void *castOrDetach(Type t);
};

template <typename T>
void qVariantSet(QCoreVariant &v, const T &t, const char *typeName)
{
    v = QCoreVariant(qRegisterMetaType<T>(typeName), &t);
}

template <typename T>
bool qVariantGet(const QCoreVariant &v, T &t, const char *typeName)
{
    if (qstrcmp(v.typeName(), typeName) != 0)
        return false;
    t = *static_cast<const T *>(v.constData());
    return true;
}

typedef QList<QCoreVariant> QCoreVariantList;
typedef QMap<QString, QCoreVariant> QCoreVariantMap;

inline QCoreVariant::QCoreVariant() {}
inline bool QCoreVariant::isValid() const { return d.type != Invalid; }

#ifdef QT_COMPAT
inline int &QCoreVariant::asInt()
{ return *static_cast<int *>(castOrDetach(Int)); }
inline uint &QCoreVariant::asUInt()
{ return *static_cast<uint *>(castOrDetach(UInt)); }
inline Q_LONGLONG &QCoreVariant::asLongLong()
{ return *static_cast<Q_LONGLONG *>(castOrDetach(LongLong)); }
inline Q_ULONGLONG &QCoreVariant::asULongLong()
{ return *static_cast<Q_ULONGLONG *>(castOrDetach(ULongLong)); }
inline bool &QCoreVariant::asBool()
{ return *static_cast<bool *>(castOrDetach(Bool)); }
inline double &QCoreVariant::asDouble()
{ return *static_cast<double *>(castOrDetach(Double)); }
inline QByteArray& QCoreVariant::asByteArray()
{ return *static_cast<QByteArray *>(castOrDetach(ByteArray)); }
inline QBitArray& QCoreVariant::asBitArray()
{ return *static_cast<QBitArray *>(castOrDetach(BitArray)); }
inline QString& QCoreVariant::asString()
{ return *static_cast<QString *>(castOrDetach(String)); }
inline QStringList& QCoreVariant::asStringList()
{ return *static_cast<QStringList *>(castOrDetach(StringList)); }
inline QDate& QCoreVariant::asDate()
{ return *static_cast<QDate *>(castOrDetach(Date)); }
inline QTime& QCoreVariant::asTime()
{ return *static_cast<QTime *>(castOrDetach(Time)); }
inline QDateTime& QCoreVariant::asDateTime()
{ return *static_cast<QDateTime *>(castOrDetach(DateTime)); }
#ifndef QT_NO_TEMPLATE_VARIANT
inline QList<QCoreVariant>& QCoreVariant::asList()
{ return *static_cast<QList<QCoreVariant> *>(castOrDetach(List)); }
inline QMap<QString, QCoreVariant>& QCoreVariant::asMap()
{ return *static_cast<QMap<QString, QCoreVariant> *>(castOrDetach(Map)); }
#endif
inline QPoint &QCoreVariant::asPoint()
{ return *static_cast<QPoint *>(castOrDetach(Point)); }
inline QRect &QCoreVariant::asRect()
{ return *static_cast<QRect *>(castOrDetach(Rect)); }
inline QSize &QCoreVariant::asSize()
{ return *static_cast<QSize *>(castOrDetach(Size)); }
#endif //QT_COMPAT

#ifndef QT_NO_DATASTREAM
Q_CORE_EXPORT QDataStream& operator>> (QDataStream& s, QCoreVariant& p);
Q_CORE_EXPORT QDataStream& operator<< (QDataStream& s, const QCoreVariant& p);
Q_CORE_EXPORT QDataStream& operator>> (QDataStream& s, QCoreVariant::Type& p);
Q_CORE_EXPORT QDataStream& operator<< (QDataStream& s, const QCoreVariant::Type p);
#endif

inline bool QCoreVariant::isDetached() const
{ return !d.is_shared || d.data.shared->ref == 1; }

#if defined Q_CC_MSVC && _MSC_VER < 1300
template <class T> inline T QVariant_to_helper(const QCoreVariant &v, const T *);

template <class T>
inline T QVariant_to(const QCoreVariant &v)
{
    return QVariant_to_helper<T>(v, (T *)0);
}

template<> inline int QVariant_to_helper<int>(const QCoreVariant &v, const int*) { return v.toInt(); }
template<> inline uint QVariant_to_helper<uint>(const QCoreVariant &v, const uint*) { return v.toUInt(); }
template<> inline Q_LONGLONG QVariant_to_helper<Q_LONGLONG>(const QCoreVariant &v, const Q_LONGLONG*) { return v.toLongLong(); }
template<> inline Q_ULONGLONG QVariant_to_helper<Q_ULONGLONG>(const QCoreVariant &v, const Q_ULONGLONG*) { return v.toULongLong(); }
template<> inline bool QVariant_to_helper<bool>(const QCoreVariant &v, const bool*) { return v.toBool(); }
template<> inline double QVariant_to_helper<double>(const QCoreVariant &v, const double*) { return v.toDouble(); }
template<> inline QByteArray QVariant_to_helper<QByteArray>(const QCoreVariant &v, const QByteArray*) { return v.toByteArray(); }
template<> inline QChar QVariant_to_helper<QChar>(const QCoreVariant &v, const QChar *)
{ return v.toChar(); }

template<> QBitArray QVariant_to_helper<QBitArray>(const QCoreVariant &v, const QBitArray*);
template<> QString QVariant_to_helper<QString>(const QCoreVariant &v, const QString*);
template<> QStringList QVariant_to_helper<QStringList>(const QCoreVariant &v, const QStringList*);
template<> QDate QVariant_to_helper<QDate>(const QCoreVariant &v, const QDate*);
template<> QTime QVariant_to_helper<QTime>(const QCoreVariant &v, const QTime*);
template<> QDateTime QVariant_to_helper<QDateTime>(const QCoreVariant &v, const QDateTime*);
#ifndef QT_NO_TEMPLATE_VARIANT
template<> QList<QCoreVariant>
QVariant_to_helper<QList<QCoreVariant> >(const QCoreVariant &v, const QList<QCoreVariant>*);
template<> QMap<QString,QCoreVariant>
QVariant_to_helper<QMap<QString,QCoreVariant> >(const QCoreVariant &v, const QMap<QString,QCoreVariant>*);
template<> QPoint QVariant_to_helper<QPoint>(const QCoreVariant &v, const QPoint*);
template<> QRect QVariant_to_helper<QRect>(const QCoreVariant &v, const QRect*);
template<> QSize QVariant_to_helper<QSize>(const QCoreVariant &v, const QSize*);
template<> QUrl QVariant_to_helper<QUrl>(const QCoreVariant &v, const QUrl*);
#endif

#else

template<typename T> T QVariant_to(const QCoreVariant &v);
template<> inline int QVariant_to<int>(const QCoreVariant &v) { return v.toInt(); }
template<> inline uint QVariant_to<uint>(const QCoreVariant &v) { return v.toUInt(); }
template<> inline Q_LONGLONG QVariant_to<Q_LONGLONG>(const QCoreVariant &v)
{ return v.toLongLong(); }
template<> inline Q_ULONGLONG QVariant_to<Q_ULONGLONG>(const QCoreVariant &v) { return v.toULongLong(); }
template<> inline bool QVariant_to<bool>(const QCoreVariant &v) { return v.toBool(); }
template<> inline double QVariant_to<double>(const QCoreVariant &v) { return v.toDouble(); }
template<> inline QByteArray QVariant_to<QByteArray>(const QCoreVariant &v) { return v.toByteArray(); }
template<> inline QChar QVariant_to<QChar>(const QCoreVariant &v) { return v.toChar(); }

template<> QBitArray QVariant_to<QBitArray>(const QCoreVariant &v);
template<> QString QVariant_to<QString>(const QCoreVariant &v);
template<> QStringList QVariant_to<QStringList>(const QCoreVariant &v);
template<> QDate QVariant_to<QDate>(const QCoreVariant &v);
template<> QTime QVariant_to<QTime>(const QCoreVariant &v);
template<> QDateTime QVariant_to<QDateTime>(const QCoreVariant &v);
#ifndef QT_NO_TEMPLATE_VARIANT
template<> QList<QCoreVariant> QVariant_to<QList<QCoreVariant> >(const QCoreVariant &v);
template<> QMap<QString,QCoreVariant> QVariant_to<QMap<QString,QCoreVariant> >(const QCoreVariant &v);
#endif
template<> QPoint QVariant_to<QPoint>(const QCoreVariant &v);
template<> QRect QVariant_to<QRect>(const QCoreVariant &v);
template<> QSize QVariant_to<QSize>(const QCoreVariant &v);
template<> QUrl QVariant_to<QUrl>(const QCoreVariant &v);
#endif

Q_DECLARE_SHARED(QCoreVariant);
Q_DECLARE_TYPEINFO(QCoreVariant, Q_MOVABLE_TYPE);

#ifndef QT_NO_DEBUG_OUTPUT
Q_CORE_EXPORT QDebug operator<<(QDebug, const QCoreVariant &);
#endif

#endif //QT_NO_VARIANT
#endif // QCOREVARIANT_H

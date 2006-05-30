/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QMETATYPE_H
#define QMETATYPE_H

#include <QtCore/qglobal.h>
#include <QtCore/qatomic.h>

#ifndef QT_NO_DATASTREAM
#include <QtCore/qdatastream.h>
#endif

#ifdef Bool
#error qmetatype.h must be included before any header file that defines Bool
#endif

QT_BEGIN_HEADER

QT_MODULE(Core)

class Q_CORE_EXPORT QMetaType {
public:
    enum Type {
        // these are merged with QVariant
        Void = 0, Bool = 1, Int = 2, UInt = 3, Double = 6, QChar = 7,
        QString = 10, QByteArray = 12,

        VoidStar = 128, Long, Short, Char, ULong,
        UShort, UChar, Float, QObjectStar, QWidgetStar,
        User = 256
    };

    typedef void (*Destructor)(void *);
    typedef void *(*Constructor)(const void *);

#ifndef QT_NO_DATASTREAM
    typedef void (*SaveOperator)(QDataStream &, const void *);
    typedef void (*LoadOperator)(QDataStream &, void *);
    static void registerStreamOperators(const char *typeName, SaveOperator saveOp,
                                        LoadOperator loadOp);
    static int registerType(const char *typeName, Destructor destructor, Constructor constructor,
                            SaveOperator saveOp, LoadOperator loadOp);
#endif
    static int registerType(const char *typeName, Destructor destructor,
                            Constructor constructor);
    static int type(const char *typeName);
    static const char *typeName(int type);
    static bool isRegistered(int type);
    static void *construct(int type, const void *copy);
    static void destroy(int type, void *data);

#ifndef QT_NO_DATASTREAM
    static bool save(QDataStream &stream, int type, const void *data);
    static bool load(QDataStream &stream, int type, void *data);
#endif
};

template <typename T>
void qMetaTypeDeleteHelper(T *t)
{
    delete t;
}

template <typename T>
void *qMetaTypeConstructHelper(const T *t)
{
    if (!t)
        return new T;
    return new T(*static_cast<const T*>(t));
}

#ifndef QT_NO_DATASTREAM
template <typename T>
void qMetaTypeSaveHelper(QDataStream &stream, const T *t)
{
    stream << *t;
}

template <typename T>
void qMetaTypeLoadHelper(QDataStream &stream, T *t)
{
    stream >> *t;
}
#endif // QT_NO_DATASTREAM

#include <QtCore/qmetatypedetect.h>

template <typename T>
int qRegisterMetaType(const char *typeName
#ifndef qdoc
    , T * /* dummy */ = 0
#endif
)
{
    typedef void*(*ConstructPtr)(const T*);
    ConstructPtr cptr = qMetaTypeConstructHelper<T>;
    typedef void(*DeletePtr)(T*);
    DeletePtr dptr = qMetaTypeDeleteHelper<T>;

#ifdef QT_NO_DATASTREAM
    return QMetaType::registerType(typeName, reinterpret_cast<QMetaType::Destructor>(dptr),
                                   reinterpret_cast<QMetaType::Constructor>(cptr));
#else
    return QMetaType::registerType(typeName, reinterpret_cast<QMetaType::Destructor>(dptr),
                                   reinterpret_cast<QMetaType::Constructor>(cptr),
                                   QtInternal::getSaveOperator<T>(),
                                   QtInternal::getLoadOperator<T>());
#endif
}

#ifndef QT_NO_DATASTREAM
template <typename T>
void qRegisterMetaTypeStreamOperators(const char *typeName
#ifndef qdoc
    , T * /* dummy */ = 0
#endif
)
{
    typedef void(*SavePtr)(QDataStream &, const T *);
    typedef void(*LoadPtr)(QDataStream &, T *);
    SavePtr sptr = qMetaTypeSaveHelper<T>;
    LoadPtr lptr = qMetaTypeLoadHelper<T>;

    qRegisterMetaType<T>(typeName);
    QMetaType::registerStreamOperators(typeName, reinterpret_cast<QMetaType::SaveOperator>(sptr),
                                       reinterpret_cast<QMetaType::LoadOperator>(lptr));
}
#endif

template <typename T>
struct QMetaTypeId
{
    enum { Defined = 0 };
};

template <typename T>
inline int qMetaTypeId(
#ifndef qdoc
    T * /* dummy */ = 0
#endif
)
{
    return QMetaTypeId<T>::qt_metatype_id();
}

template <typename T>
inline int qRegisterMetaType(
#ifndef qdoc
    T * dummy = 0
#endif
)
{
    return qMetaTypeId(dummy);
}

#define Q_DECLARE_METATYPE(TYPE) \
template <> \
struct QMetaTypeId< TYPE > \
{ \
    enum { Defined = 1 }; \
    static int qt_metatype_id() \
    { \
        static QBasicAtomic id = Q_ATOMIC_INIT(0); \
        if (!id) \
           id = qRegisterMetaType< TYPE >(#TYPE); \
       return id; \
    } \
};

class QString;
template<> struct QMetaTypeId<QString>
{ enum { Defined = 1 };
  static inline int qt_metatype_id() { return QMetaType::QString; } };
template<> struct QMetaTypeId<int>
{ enum { Defined = 1 };
  static inline int qt_metatype_id() { return QMetaType::Int; } };
template<> struct QMetaTypeId<uint>
{ enum { Defined = 1 };
  static inline int qt_metatype_id() { return QMetaType::UInt; } };
template<> struct QMetaTypeId<bool>
{ enum { Defined = 1 };
  static inline int qt_metatype_id() { return QMetaType::Bool; } };
template<> struct QMetaTypeId<double>
{ enum { Defined = 1 };
  static inline int qt_metatype_id() { return QMetaType::Double; } };
class QByteArray;
template<> struct QMetaTypeId<QByteArray>
{ enum { Defined = 1 };
  static inline int qt_metatype_id() { return QMetaType::QByteArray; } };
class QChar;
template<> struct QMetaTypeId<QChar>
{ enum { Defined = 1 };
  static inline int qt_metatype_id() { return QMetaType::QChar; } };
template<> struct QMetaTypeId<void>
{ enum { Defined = 1 };
  static inline int qt_metatype_id() { return QMetaType::Void; } };
template<> struct QMetaTypeId<long>
{ enum { Defined = 1 };
  static inline int qt_metatype_id() { return QMetaType::Long; } };
template<> struct QMetaTypeId<short>
{ enum { Defined = 1 };
  static inline int qt_metatype_id() { return QMetaType::Short; } };
template<> struct QMetaTypeId<char>
{ enum { Defined = 1 };
  static inline int qt_metatype_id() { return QMetaType::Char; } };
template<> struct QMetaTypeId<ulong>
{ enum { Defined = 1 };
  static inline int qt_metatype_id() { return QMetaType::ULong; } };
template<> struct QMetaTypeId<ushort>
{ enum { Defined = 1 };
  static inline int qt_metatype_id() { return QMetaType::UShort; } };
template<> struct QMetaTypeId<uchar>
{ enum { Defined = 1 };
  static inline int qt_metatype_id() { return QMetaType::UChar; } };
template<> struct QMetaTypeId<float>
{ enum { Defined = 1 };
  static inline int qt_metatype_id() { return QMetaType::Float; } };
class QObject;
template<> struct QMetaTypeId<QObject *>
{ enum { Defined = 1 };
  static inline int qt_metatype_id() { return QMetaType::QObjectStar; } };
class QWidget;
template<> struct QMetaTypeId<QWidget *>
{ enum { Defined = 1 };
  static inline int qt_metatype_id() { return QMetaType::QWidgetStar; } };
template<> struct QMetaTypeId<void *>
{ enum { Defined = 1 };
  static inline int qt_metatype_id() { return QMetaType::VoidStar; } };

QT_END_HEADER

#endif // QMETATYPE_H

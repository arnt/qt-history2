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

#ifndef QMETATYPE_H
#define QMETATYPE_H

#include "QtCore/qdatastream.h"

#ifdef Bool
#error qmetatype.h must be included before any header file that define Bool
#endif

class Q_CORE_EXPORT QMetaType {
public:
    enum {
        // these are merged with QCoreVariant
        Void = 0, QString = 3, Int = 16, UInt = 17,
        Bool = 18, Double = 19, QByteArray = 29, QChar = 35,

        VoidStar = 64, Long, Short, Char, ULong,
        UShort, UChar, Float,
        User = 256
    };

    typedef void (*Destructor)(void *);
    typedef void *(*Constructor)(const void *);
    typedef void (*SaveOperator)(QDataStream &, const void *);
    typedef void (*LoadOperator)(QDataStream &, void *);

    static int registerType(const char *typeName, Destructor destructor,
                            Constructor constructor);
    static void registerStreamOperators(const char *typeName, SaveOperator saveOp,
                                        LoadOperator loadOp);
    static int type(const char *typeName);
    static const char *typeName(int type);
    static bool isRegistered(int type);
    static void *construct(int type, const void *copy);
    static void destroy(int type, void *data);
    static bool save(QDataStream &stream, int type, const void *data);
    static bool load(QDataStream &stream, int type, void *data);
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

template <typename T>
static int qRegisterMetaType(const char *typeName, T * = 0)
{
    typedef void*(*ConstructPtr)(const T*);
    ConstructPtr cptr = qMetaTypeConstructHelper<T>;
    typedef void(*DeletePtr)(T*);
    DeletePtr dptr = qMetaTypeDeleteHelper<T>;

    return QMetaType::registerType(typeName, reinterpret_cast<QMetaType::Destructor>(dptr),
                                   reinterpret_cast<QMetaType::Constructor>(cptr));
}

template <typename T>
static void qRegisterMetaTypeStreamOperators(const char *typeName, T * = 0)
{
    typedef void(*SavePtr)(QDataStream &, const T *);
    typedef void(*LoadPtr)(QDataStream &, T *);
    SavePtr sptr = qMetaTypeSaveHelper<T>;
    LoadPtr lptr = qMetaTypeLoadHelper<T>;

    QMetaType::registerStreamOperators(typeName, reinterpret_cast<QMetaType::SaveOperator>(sptr),
                                       reinterpret_cast<QMetaType::LoadOperator>(lptr));
}

/* helper class for compile time asserts */
template<class T>
class UNDECLARED_METATYPE
{
    /*
       If you get compile errors about this constructor being private,
       please read about Q_DECLARE_METATYPE
     */
    UNDECLARED_METATYPE() {}
};

template <class T>
static int qt_metatype_id(T * = 0) { UNDECLARED_METATYPE<T> tp; return QMetaType::Void; }

#define Q_DECLARE_METATYPE(TYPE) \
template<> static int qt_metatype_id(TYPE *) \
{ \
    static int tp = qRegisterMetaType<TYPE>(#TYPE); \
    return tp; \
}

template<> static inline int qt_metatype_id(QString *) { return QMetaType::QString; }
template<> static inline int qt_metatype_id(int *) { return QMetaType::Int; }
template<> static inline int qt_metatype_id(uint *) { return QMetaType::UInt; }
template<> static inline int qt_metatype_id(bool *) { return QMetaType::Bool; }
template<> static inline int qt_metatype_id(double *) { return QMetaType::Double; }
template<> static inline int qt_metatype_id(QByteArray *) { return QMetaType::QByteArray; }
template<> static inline int qt_metatype_id(QChar *) { return QMetaType::QChar; }
template<> static inline int qt_metatype_id(void **) { return QMetaType::VoidStar; }
template<> static inline int qt_metatype_id(long *) { return QMetaType::Long; }
template<> static inline int qt_metatype_id(short *) { return QMetaType::Short; }
template<> static inline int qt_metatype_id(char *) { return QMetaType::Char; }
template<> static inline int qt_metatype_id(ulong *) { return QMetaType::ULong; }
template<> static inline int qt_metatype_id(ushort *) { return QMetaType::UShort; }
template<> static inline int qt_metatype_id(uchar *) { return QMetaType::UChar; }
template<> static inline int qt_metatype_id(float *) { return QMetaType::Float; }

#endif // QMETATYPE_H

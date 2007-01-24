/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSCRIPTENGINE_H
#define QSCRIPTENGINE_H

#include <QtCore/qmetatype.h>
#include <QtCore/qvariant.h>

#ifndef QT_NO_QOBJECT
#include <QtCore/qobject.h>
#else
#include <QtCore/qobjectdefs.h>
#endif

#include <QtScript/qscriptvalue.h>
#include <QtScript/qscriptcontext.h>

QT_BEGIN_HEADER

QT_MODULE(Script)

class QDateTime;
class QScriptEnginePrivate;
class QScriptNameIdImpl;
class QScriptCustomTypeInfo;

class Q_SCRIPT_EXPORT QScriptNameId
{
    friend class QScriptEnginePrivate;

public:
    inline QScriptNameId():
        m_id(0) {}

    inline QScriptNameId(const QScriptNameId &other):
        m_id(other.m_id) {}

    inline ~QScriptNameId()                                     { m_id = 0; }

    inline bool isValid() const                                 { return (m_id != 0); }
    inline bool operator==(const QScriptNameId &other) const    { return m_id == other.m_id; }
    inline bool operator!=(const QScriptNameId &other) const    { return !(*this == other); }

    inline operator QScriptNameIdImpl*() const                  { return m_id; }

private:
    inline QScriptNameId(QScriptNameIdImpl *id) : m_id(id) {}

    QScriptNameIdImpl *m_id;
};

#ifndef QT_NO_QOBJECT

template <class T>
static inline QScriptValue qscriptQClassConstructor(QScriptContext *, QScriptEngine *)
{
    return 0;
}

template <class T>
inline QScriptValue qScriptValueFromQClass(QScriptEngine *engine);

#endif // QT_NO_QOBJECT

typedef QScriptValue (*QScriptFunctionSignature)(QScriptContext *, QScriptEngine *);

#ifndef QT_NO_REGEXP
class QRegExp;
#endif

class Q_SCRIPT_EXPORT QScriptEngine
#ifndef QT_NO_QOBJECT
    : public QObject
#endif
{
#ifndef QT_NO_QOBJECT
    Q_OBJECT
#endif
public:
    QScriptEngine();
#ifndef QT_NO_QOBJECT
    explicit QScriptEngine(QObject *parent);
#endif
    virtual ~QScriptEngine();

    QScriptValue globalObject() const;
    QScriptContext *currentContext() const;

    bool canEvaluate(const QString &program) const;

    QScriptValue evaluate(const QString &program);
    QScriptValue evaluate(const QString &program, int lineNumber);

    bool hasUncaughtException() const;
    int uncaughtExceptionLineNumber() const;

    QScriptValue nullValue();
    QScriptValue undefinedValue();

    Q_DECL_DEPRECATED QScriptValue nullScriptValue();
    Q_DECL_DEPRECATED QScriptValue undefinedScriptValue();

    QScriptValue scriptValue(bool value);
    QScriptValue scriptValue(int value);
    QScriptValue scriptValue(uint value);
    QScriptValue scriptValue(qlonglong value);
    QScriptValue scriptValue(qulonglong value);
    QScriptValue scriptValue(qnumber value);
    QScriptValue scriptValue(const QString &value);

#ifndef QT_NO_CAST_FROM_ASCII
    QT_ASCII_CAST_WARN QScriptValue scriptValue(const char *value);
#endif

    QScriptValue newFunction(QScriptFunctionSignature signature, int length = 0);
    QScriptValue newFunction(QScriptFunctionSignature signature, const QScriptValue &prototype, int length = 0);
    QScriptValue newVariant(const QVariant &value);

    Q_DECL_DEPRECATED QScriptValue scriptValue(QScriptFunctionSignature signature, int length = 0);
    Q_DECL_DEPRECATED QScriptValue scriptValue(QScriptFunctionSignature signature, const QScriptValue &prototype, int length = 0);
    Q_DECL_DEPRECATED QScriptValue scriptValueFromVariant(const QVariant &value);

#ifndef QT_NO_REGEXP
    QScriptValue newRegExp(const QRegExp &regexp);
    Q_DECL_DEPRECATED QScriptValue scriptValue(const QRegExp &regexp);
#endif

    QScriptValue newObject();
    QScriptValue newArray(uint length = 0);
    QScriptValue newRegExp(const QString &pattern, const QString &flags);
    QScriptValue newDate(qnumber value);
    QScriptValue newDate(const QDateTime &value);

    QScriptValue defaultPrototype(int metaTypeId) const;
    void setDefaultPrototype(int metaTypeId, const QScriptValue &prototype);

#ifndef QT_NO_QOBJECT
    QScriptValue newQObject(QObject *object);

    QScriptValue newQMetaObject(const QMetaObject *metaObject,
                                const QScriptValue &ctor = QScriptValue());

    Q_DECL_DEPRECATED QScriptValue scriptValueFromQObject(QObject *object);

    Q_DECL_DEPRECATED QScriptValue scriptValue(const QMetaObject *metaObject,
                                               const QScriptValue &ctor);

#  ifndef QT_NO_MEMBER_TEMPLATES
    template <class T> QScriptValue scriptValueFromQClass()
    {
        return qScriptValueFromQClass<T>(this);
    }
#  endif // QT_NO_MEMBER_TEMPLATES
#endif // QT_NO_QOBJECT

    QScriptNameId nameId(const QString &value);

#ifndef QT_NO_MEMBER_TEMPLATES
    template <typename T>
    inline QScriptValue scriptValueFromValue(const T &value)
    {
        return qScriptValueFromValue(this, value);
    }
#endif // QT_NO_MEMBER_TEMPLATES

    typedef QScriptValue (*MarshalFunction)(QScriptEngine *, const void *);
    typedef void (*DemarshalFunction)(const QScriptValue &, void *);

private:
    QScriptValue create(int type, const void *ptr);

    bool convert(const QScriptValue &value, int type, void *ptr);

    void registerCustomType(int type, MarshalFunction mf,
                            DemarshalFunction df,
                            const QScriptValue &prototype);

    friend inline void qScriptRegisterMetaType_helper(QScriptEngine *,
        int, MarshalFunction, DemarshalFunction, const QScriptValue &);

    friend inline QScriptValue qScriptValueFromValue_helper(QScriptEngine *, int, const void *);

    friend inline bool qscript_cast_helper(const QScriptValue &, int, void *);

protected:
#ifdef QT_NO_QOBJECT
    QScriptEnginePrivate *d_ptr;

    QScriptEngine(QScriptEnginePrivate &dd);
#else
    QScriptEngine(QScriptEnginePrivate &dd, QObject *parent = 0);
#endif

private:
    Q_DECLARE_PRIVATE(QScriptEngine)
    Q_DISABLE_COPY(QScriptEngine)
};

#ifndef QT_NO_QOBJECT
template <class T>
inline QScriptValue qScriptValueFromQClass(QScriptEngine *engine)
{
    return engine->newQMetaObject(&T::staticMetaObject,
                                  engine->newFunction(qscriptQClassConstructor<T>));
}

#define Q_SCRIPT_DECLARE_QCLASS(T, _Arg1) \
template<> static inline QScriptValue qscriptQClassConstructor<T>(QScriptContext *ctx, QScriptEngine *eng) \
{ \
    _Arg1 arg1 = qscript_cast<_Arg1> (ctx->argument(0)); \
    return eng->newQObject(new T(arg1)); \
}

#endif // QT_NO_QOBJECT

inline QScriptValue qScriptValueFromValue_helper(QScriptEngine *engine, int type, const void *ptr)
{
    if (!engine)
        return QScriptValue();
    return engine->create(type, ptr);
}

template <typename T>
inline QScriptValue qScriptValueFromValue(QScriptEngine *engine, const T &t)
{
    return qScriptValueFromValue_helper(engine, qMetaTypeId<T>(), &t);
}

inline bool qscript_cast_helper(const QScriptValue &value, int type, void *ptr)
{
    QScriptEngine *eng = value.engine();
    if (!eng)
        return false;
    return eng->convert(value, type, ptr);
}

template<typename T>
T qscript_cast(const QScriptValue &value
#ifndef Q_QDOC
, T * = 0
#endif
    )
{
    T t;
    const int id = qMetaTypeId<T>();
    if (qscript_cast_helper(value, id, &t))
        return t;
    if (value.isVariant())
        return qvariant_cast<T>(value.toVariant());
    return T();
}

inline void qScriptRegisterMetaType_helper(QScriptEngine *eng, int type,
                                           QScriptEngine::MarshalFunction mf,
                                           QScriptEngine::DemarshalFunction df,
                                           const QScriptValue &prototype)
{
    eng->registerCustomType(type, mf, df, prototype);
}

template<typename T>
int qScriptRegisterMetaType(
    QScriptEngine *eng,
    QScriptValue (*toValue)(QScriptEngine *, const T &t),
    void (*fromValue)(const QScriptValue &, T &t),
    const QScriptValue &prototype = QScriptValue()
#ifndef qdoc
    , T * /* dummy */ = 0
#endif
)
{
    const int id = qRegisterMetaType<T>(); // make sure it's registered
    qScriptRegisterMetaType_helper(
        eng, id, reinterpret_cast<QScriptEngine::MarshalFunction>(toValue),
        reinterpret_cast<QScriptEngine::DemarshalFunction>(fromValue),
        prototype);
    return id;
}

QT_END_HEADER

#endif // QSCRIPTENGINE_H

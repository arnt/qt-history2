#ifndef QTSCRIPTEXTENSIONS_GLOBAL_H
#define QTSCRIPTEXTENSIONS_GLOBAL_H

#include <QtCore/QSharedData>

#define DECLARE_SELF(T, __fn__) \
    Q##T* self = qscriptvalue_cast<Q##T*>(ctx->thisObject()); \
    if (!self) { \
        return ctx->throwError(QScriptContext::TypeError, \
            QString::fromLatin1("%0.prototype.%1: this object is not a %0") \
            .arg(#T).arg(#__fn__)); \
    }

#define DECLARE_SELF2(T, __fn__, __ret__) \
    Q##T* self = qscriptvalue_cast<Q##T*>(thisObject()); \
    if (!self) { \
        context()->throwError(QScriptContext::TypeError, \
            QString::fromLatin1("%0.prototype.%1: this object is not a %0") \
            .arg(#T).arg(#__fn__)); \
        return __ret__; \
    }

#define ADD_PROTO_FUNCTION(__p__, __f__) \
    __p__.setProperty(#__f__, __p__.engine()->newFunction(__f__))

#define ADD_CTOR_FUNCTION(__c__, __f__) ADD_PROTO_FUNCTION(__c__, __f__)

#define ADD_ENUM_VALUE(__c__, __ns__, __v__) \
    __c__.setProperty(#__v__, QScriptValue(__c__.engine(), __ns__::__v__))

namespace QScript
{

enum {
    UserOwnership = 1
};

template <typename T>
class Finalizer
{
public:
    void finalize(T &, uint)
    { /* do nothing */ }
};

template <typename T>
class Finalizer<T *>
{
public:
    void finalize(T *ptr, uint flags)
    {
        if (!(flags & UserOwnership))
            delete ptr;
    }
};

template <typename T, class Finalizer = Finalizer<T> >
class Wrapper : public QSharedData
{
public:
    typedef T value_type;
    typedef QExplicitlySharedDataPointer<Wrapper<T> > pointer_type;

    virtual ~Wrapper()
    {
        Finalizer f;
        f.finalize(m_value, m_flags);
    }

    T &value()
    {
        return m_value;
    }

    operator T()
    {
        return m_value;
    }

    static pointer_type wrap(const T &value, uint flags = 0)
    {
        return pointer_type(new Wrapper(value, flags));
    }

    static QScriptValue toScriptValue(QScriptEngine *engine, T const &source)
    {
        if (!source)
            return engine->nullValue();
        return engine->newVariant(qVariantFromValue(source));
    }

    static void fromScriptValue(const QScriptValue &value, T &target)
    {
        if (value.isVariant()) {
            QVariant var = value.toVariant();
            if (qVariantCanConvert<T>(var)) {
                target = qvariant_cast<T>(var);
            } else if (qVariantCanConvert<pointer_type>(var)) {
                target = qvariant_cast<pointer_type>(var)->operator T();
            } else {
                // look in prototype chain
                target = 0;
                int type = qMetaTypeId<T>();
                int pointerType = qMetaTypeId<pointer_type>();
                QScriptValue proto = value.prototype();
                while (proto.isObject() && proto.isVariant()) {
                    int protoType = proto.toVariant().userType();
                    if ((type == protoType) || (pointerType == protoType)) {
                        QByteArray name = QMetaType::typeName(var.userType());
                        if (name.startsWith("QScript::Wrapper<")) {
                            target = (*reinterpret_cast<pointer_type*>(var.data()))->operator T();
                            break;
                        } else {
                            target = static_cast<T>(var.data());
                            break;
                        }
                    }
                    proto = proto.prototype();
                }
            }
        } else if (value.isQObject()) {
            QObject *qobj = value.toQObject();
            QByteArray typeName = QMetaType::typeName(qMetaTypeId<T>());
            target = reinterpret_cast<T>(qobj->qt_metacast(typeName.left(typeName.size()-1)));
        } else {
            target = 0;
        }
    }

    uint flags() const
    { return m_flags; }
    void setFlags(uint flags)
    { m_flags = flags; }
    void unsetFlags(uint flags)
    { m_flags &= ~flags; }

protected:
    Wrapper(const T &value, uint flags)
        : m_flags(flags), m_value(value)
    {
    }

private:
    uint m_flags;
    T m_value;
};

template <typename W>
int registerMetaTypeWrapper(
    QScriptEngine *eng,
    const QScriptValue &prototype = QScriptValue(),
    typename W::value_type * /* dummy */ = 0
)
{
    QScriptValue (*mf)(QScriptEngine *, typename W::value_type const &) = W::toScriptValue;
    void (*df)(const QScriptValue &, typename W::value_type &) = W::fromScriptValue;
    const int id = qMetaTypeId<typename W::value_type>();
    qScriptRegisterMetaType_helper(
        eng, id, reinterpret_cast<QScriptEngine::MarshalFunction>(mf),
        reinterpret_cast<QScriptEngine::DemarshalFunction>(df),
        prototype);
    eng->setDefaultPrototype(qMetaTypeId<typename W::pointer_type>(), prototype);
    return id;
}

inline void maybeReleaseOwnership(const QScriptValue &value)
{
    if (value.isVariant()) {
        QVariant var = value.toVariant();
        QByteArray name = QMetaType::typeName(var.userType());
        if (name.startsWith("QScript::Wrapper<"))
            (*reinterpret_cast<QScript::Wrapper<void*>::pointer_type *>(var.data()))->setFlags(UserOwnership);
    }
}

inline void maybeTakeOwnership(const QScriptValue &value)
{
    if (value.isVariant()) {
        QVariant var = value.toVariant();
        QByteArray name = QMetaType::typeName(var.userType());
        if (name.startsWith("QScript::Wrapper<"))
            (*reinterpret_cast<QScript::Wrapper<void*>::pointer_type *>(var.data()))->unsetFlags(UserOwnership);
    }
}

template <class T>
QScriptValue construct(QScriptEngine *eng, T *item)
{
    uint flags = item->parentItem() ? UserOwnership : 0;
    return eng->newVariant(qVariantFromValue(QScript::Wrapper<T*>::wrap(item, flags)));
}

} // namespace QScript

#endif

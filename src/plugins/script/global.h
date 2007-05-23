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

#define DECLARE_POINTER_METATYPE(T) \
    Q_DECLARE_METATYPE(T*); \
    Q_DECLARE_METATYPE(QScript::Pointer<T>::wrapped_pointer_type);

namespace QScript
{

enum {
    UserOwnership = 1
};

template <typename T>
class Pointer : public QSharedData
{
public:
    typedef T* pointer_type;
    typedef QExplicitlySharedDataPointer<Pointer<T> > wrapped_pointer_type;

    ~Pointer()
    {
        if (!(m_flags & UserOwnership))
            delete m_value;
    }

    operator T*()
    {
        return m_value;
    }

    operator const T*() const
    {
        return m_value;
    }

    static wrapped_pointer_type create(T *value, uint flags = 0)
    {
        return wrapped_pointer_type(new Pointer(value, flags));
    }

    static QScriptValue toScriptValue(QScriptEngine *engine, T* const &source)
    {
        if (!source)
            return engine->nullValue();
        return engine->newVariant(qVariantFromValue(source));
    }

    static void fromScriptValue(const QScriptValue &value, T* &target)
    {
        if (value.isVariant()) {
            QVariant var = value.toVariant();
            if (qVariantCanConvert<T*>(var)) {
                target = qvariant_cast<T*>(var);
            } else if (qVariantCanConvert<wrapped_pointer_type>(var)) {
                target = qvariant_cast<wrapped_pointer_type>(var)->operator T*();
            } else {
                // look in prototype chain
                target = 0;
                int type = qMetaTypeId<T*>();
                int pointerType = qMetaTypeId<wrapped_pointer_type>();
                QScriptValue proto = value.prototype();
                while (proto.isObject() && proto.isVariant()) {
                    int protoType = proto.toVariant().userType();
                    if ((type == protoType) || (pointerType == protoType)) {
                        QByteArray name = QMetaType::typeName(var.userType());
                        if (name.startsWith("QScript::Pointer<")) {
                            target = (*reinterpret_cast<wrapped_pointer_type*>(var.data()))->operator T*();
                            break;
                        } else {
                            target = static_cast<T*>(var.data());
                            break;
                        }
                    }
                    proto = proto.prototype();
                }
            }
        } else if (value.isQObject()) {
            QObject *qobj = value.toQObject();
            QByteArray typeName = QMetaType::typeName(qMetaTypeId<T*>());
            target = reinterpret_cast<T*>(qobj->qt_metacast(typeName.left(typeName.size()-1)));
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
    Pointer(T* value, uint flags)
        : m_flags(flags), m_value(value)
    {}

private:
    uint m_flags;
    T* m_value;
};

template <typename T>
int registerPointerMetaType(
    QScriptEngine *eng,
    const QScriptValue &prototype = QScriptValue(),
    T * /* dummy */ = 0
)
{
    QScriptValue (*mf)(QScriptEngine *, T* const &) = Pointer<T>::toScriptValue;
    void (*df)(const QScriptValue &, T* &) = Pointer<T>::fromScriptValue;
    const int id = qMetaTypeId<T*>();
    qScriptRegisterMetaType_helper(
        eng, id, reinterpret_cast<QScriptEngine::MarshalFunction>(mf),
        reinterpret_cast<QScriptEngine::DemarshalFunction>(df),
        prototype);
    eng->setDefaultPrototype(qMetaTypeId<typename Pointer<T>::wrapped_pointer_type>(), prototype);
    return id;
}

inline void maybeReleaseOwnership(const QScriptValue &value)
{
    if (value.isVariant()) {
        QVariant var = value.toVariant();
        QByteArray name = QMetaType::typeName(var.userType());
        if (name.startsWith("QScript::Pointer<"))
            (*reinterpret_cast<Pointer<void*>::wrapped_pointer_type *>(var.data()))->setFlags(UserOwnership);
    }
}

inline void maybeTakeOwnership(const QScriptValue &value)
{
    if (value.isVariant()) {
        QVariant var = value.toVariant();
        QByteArray name = QMetaType::typeName(var.userType());
        if (name.startsWith("QScript::Pointer<"))
            (*reinterpret_cast<Pointer<void*>::wrapped_pointer_type *>(var.data()))->unsetFlags(UserOwnership);
    }
}

template <class T>
inline QScriptValue wrapPointer(QScriptEngine *eng, T *ptr, uint flags = 0)
{
    return eng->newVariant(qVariantFromValue(Pointer<T>::create(ptr, flags)));
}

} // namespace QScript

#ifdef QGRAPHICSITEM_H

namespace QScript {

template <class T>
inline QScriptValue wrapGVPointer(QScriptEngine *eng, T *item)
{
    uint flags = item->parentItem() ? UserOwnership : 0;
    return wrapPointer<T>(eng, item, flags);
}

} // namespace QScript

#endif // QGRAPHICSITEM_H

#endif // QTSCRIPTEXTENSIONS_GLOBAL_H

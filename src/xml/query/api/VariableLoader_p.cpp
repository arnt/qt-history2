/****************************************************************************
 * **
 * ** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
 * **
 * ** This file is part of the Patternist project on Trolltech Labs.
 * **
 * ** $TROLLTECH_GPL_LICENSE$
 * **
 * ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * **
 * ****************************************************************************/

#include <QVariant>
#include <QStringList>

#include "CommonSequenceTypes.h"
#include "Integer.h"
#include "Item.h"
#include "ListIterator.h"
#include "SequenceType.h"
#include "AtomicString.h"
#include "VariableLoader_p.h"

class VariantListIterator : public Patternist::ListIteratorPlatform<QVariant, Patternist::Item, VariantListIterator>
{
public:
    inline VariantListIterator(const QVariantList &list,
                               const Patternist::QObjectNodeModel::Ptr &nm) : Patternist::ListIteratorPlatform<QVariant, Patternist::Item, VariantListIterator>(list)
                                                                            , m_nodeModel(nm)
    {
    }

private:
    friend class Patternist::ListIteratorPlatform<QVariant, Patternist::Item, VariantListIterator>;

    inline Patternist::Item inputToOutputItem(const QVariant &inputType) const
    {
        return Patternist::AtomicValue::toXDM(inputType, m_nodeModel.get());
    }

    const Patternist::QObjectNodeModel::Ptr m_nodeModel;
};

class StringListIterator : public Patternist::ListIteratorPlatform<QString, Patternist::Item, StringListIterator>
{
public:
    inline StringListIterator(const QStringList &list) : Patternist::ListIteratorPlatform<QString, Patternist::Item, StringListIterator>(list)
    {
    }

private:
    friend class Patternist::ListIteratorPlatform<QString, Patternist::Item, StringListIterator>;

    static inline Patternist::Item inputToOutputItem(const QString &inputType)
    {
        return Patternist::AtomicString::fromValue(inputType);
    }
};

Patternist::SequenceType::Ptr VariableLoader::announceExternalVariable(const Patternist::QName name,
                                                                       const Patternist::SequenceType::Ptr &declaredType)
{
    Q_UNUSED(declaredType);
    const QVariant v(m_bindingHash.value(name));

    switch(v.type())
    {
        case QVariant::Char:
        /* Fallthrough. */
        case QVariant::String:
            return Patternist::CommonSequenceTypes::ExactlyOneString;
        case QVariant::Bool:
            return Patternist::CommonSequenceTypes::ExactlyOneBoolean;
        case QMetaType::QObjectStar:
            return Patternist::CommonSequenceTypes::ExactlyOneElement;
        default:
            return Patternist::SequenceType::Ptr();
    }
}

Patternist::Item::Iterator::Ptr VariableLoader::evaluateSequence(const Patternist::QName name,
                                                                 const Patternist::DynamicContext::Ptr &)
{
    const QVariant v(m_bindingHash.value(name));
    qDebug() << v.type();

    switch(v.type())
    {
        case QVariant::StringList:
            return Patternist::Item::Iterator::Ptr(new StringListIterator(v.toStringList()));
        case QVariant::List:
            return Patternist::Item::Iterator::Ptr(new VariantListIterator(v.toList(), m_nodeModel));
        default:
            return makeSingletonIterator(Patternist::AtomicValue::toXDM(v, m_nodeModel.get()));
    }
}

Patternist::Item VariableLoader::evaluateSingleton(const Patternist::QName name,
                                                   const Patternist::DynamicContext::Ptr &)
{
    return Patternist::AtomicValue::toXDM(m_bindingHash.value(name), m_nodeModel.get());
}

// vim: et:ts=4:sw=4:sts=4

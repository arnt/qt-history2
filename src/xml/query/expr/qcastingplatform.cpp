/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/**
 * @file
 * @short This file is included by CastingPlatform.h.
 * If you need includes in this file, put them in CasttingPlatform.h, outside of the namespace.
 */

template <typename TSubClass, const bool issueError>
Item CastingPlatform<TSubClass, issueError>::castWithCaster(const Item &sourceValue,
                                                                 const AtomicCaster::Ptr &caster,
                                                                 const DynamicContext::Ptr &context) const
{
    Q_ASSERT(sourceValue);
    Q_ASSERT(caster);
    Q_ASSERT(context);

    const Item retval(caster->castFrom(sourceValue, context));

    if(issueError)
    {
        if(retval.as<AtomicValue>()->hasError())
        {
            issueCastError(retval, sourceValue, context);
            return Item();
        }
        else
            return retval;
    }
    else
        return retval;
}

template <typename TSubClass, const bool issueError>
Item CastingPlatform<TSubClass, issueError>::cast(const Item &sourceValue,
                                                       const DynamicContext::Ptr &context) const
{
    Q_ASSERT(sourceValue);
    Q_ASSERT(context);
    Q_ASSERT(targetType());

    if(m_caster)
        return castWithCaster(sourceValue, m_caster, context);
    else
    {
        bool castImpossible = false;
        const AtomicCaster::Ptr caster(locateCaster(sourceValue.type(), context, castImpossible));

        if(!issueError && castImpossible)
        {
            /* If we're supposed to issue an error(issueError) then this
             * line will never be reached, because locateCaster() will in
             * that case throw. */
            return ValidationError::createError();
        }
        else
            return castWithCaster(sourceValue, caster, context);
    }
}

template <typename TSubClass, const bool issueError>
bool CastingPlatform<TSubClass, issueError>::prepareCasting(const StaticContext::Ptr &context,
                                                            const ItemType::Ptr &sourceType)
{
    Q_ASSERT(sourceType);
    Q_ASSERT(context);

    if(*sourceType == *BuiltinTypes::xsAnyAtomicType ||
       *sourceType == *BuiltinTypes::numeric)
        return true; /* The type could not be narrowed better than xs:anyAtomicType
                        or numeric at compile time. We'll do lookup at runtime instead. */

    bool castImpossible = false;
    m_caster = locateCaster(sourceType, context, castImpossible);

    return !castImpossible;
}

template <typename TSubClass, const bool issueError>
AtomicCaster::Ptr CastingPlatform<TSubClass, issueError>::locateCaster(const ItemType::Ptr &sourceType,
                                                                       const ReportContext::Ptr &context,
                                                                       bool &castImpossible) const
{
    Q_ASSERT(sourceType);
    Q_ASSERT(targetType());

    const AtomicCasterLocator::Ptr locator(static_cast<AtomicType *>(
            targetType().get())->casterLocator());
    if(!locator)
    {
        if(issueError)
        {
            context->error(tr("No casting is possible with %2 as the target type.")
                                        .arg(formatType(context->namePool(), targetType())),
                                       ReportContext::XPTY0004, static_cast<const TSubClass *>(this));
        }
        else
            castImpossible = true;

        return AtomicCaster::Ptr();
    }

    const AtomicCaster::Ptr caster(static_cast<const AtomicType *>(sourceType.get())->accept(locator, static_cast<const TSubClass *>(this)));
    if(!caster)
    {
        if(issueError)
        {
            context->error(tr("It is not possible to cast from %1 to %2.")
                                            .arg(formatType(context->namePool(), sourceType))
                                            .arg(formatType(context->namePool(), targetType())),
                                       ReportContext::XPTY0004, static_cast<const TSubClass *>(this));
        }
        else
            castImpossible = true;

        return AtomicCaster::Ptr();
    }

    return caster;
}

template <typename TSubClass, const bool issueError>
void CastingPlatform<TSubClass, issueError>::checkTargetType(const StaticContext::Ptr &context) const
{
    Q_ASSERT(context);

    const ItemType::Ptr tType(targetType());
    Q_ASSERT(tType);
    Q_ASSERT(tType->isAtomicType());
    const AtomicType::Ptr asAtomic(tType);

    /* This catches casting to xs:NOTATION and xs:anyAtomicType. */
    if(asAtomic->isAbstract())
    {
        context->error(tr("Casting to %1 is not possible because it "
                                     "is an abstract type, and can therefore never be instantiated.")
                                .arg(formatType(context->namePool(), tType)),
                          ReportContext::XPST0080,
                          static_cast<const TSubClass*>(this));
    }
}

template <typename TSubClass, const bool issueError>
void CastingPlatform<TSubClass, issueError>::issueCastError(const Item &validationError,
                                                            const Item &sourceValue,
                                                            const DynamicContext::Ptr &context) const
{
    Q_ASSERT(validationError);
    Q_ASSERT(context);
    Q_ASSERT(validationError.isAtomicValue());
    Q_ASSERT(validationError.as<AtomicValue>()->hasError());

    const ValidationError::Ptr err(validationError.as<ValidationError>());
    QString msg(err->message());

    if(msg.isNull())
    {
        msg = tr("It's not possible to cast the value %1 of type %2 to %3")
                 .arg(formatData(sourceValue.stringValue()))
                 .arg(formatType(context->namePool(), sourceValue.type()))
                 .arg(formatType(context->namePool(), targetType()));
    }
    else
    {
        Q_ASSERT(!msg.isEmpty());
        msg = tr("Failure when casting from %1 to %2: %3")
                 .arg(formatType(context->namePool(), sourceValue.type()))
                 .arg(formatType(context->namePool(), targetType()))
                 .arg(msg);
    }

    context->error(msg, err->errorCode(),
                   static_cast<const TSubClass*>(this));
}

// vim: et:ts=4:sw=4:sts=4

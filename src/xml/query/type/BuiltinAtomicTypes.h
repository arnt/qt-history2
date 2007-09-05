/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the Patternist project on Trolltech Labs.
**
** $TROLLTECH_GPL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
***************************************************************************
*/

#ifndef Patternist_BuiltinAtomicTypes_H
#define Patternist_BuiltinAtomicTypes_H

#include "AtomicCasterLocators.h"
#include "AtomicComparatorLocators.h"
#include "BuiltinAtomicType.h"

QT_BEGIN_HEADER 

namespace Patternist
{

    /**
     * @short Implements the type @c xs:anyAtomicType.
     *
     * @ingroup Patternist_types
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class AnyAtomicType : public BuiltinAtomicType
    {
    public:
        typedef PlainSharedPtr<AnyAtomicType> Ptr;

        virtual AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &visitor,
                                                    const SourceLocationReflection *const reflection) const;
        virtual AtomicTypeVisitorResult::Ptr accept(const ParameterizedAtomicTypeVisitor::Ptr &visitor,
                                                    const qint16 op,
                                                    const SourceLocationReflection *const reflection) const;
        virtual QName name(const NamePool::Ptr &np) const;
        virtual QString displayName(const NamePool::Ptr &np) const;

        /**
         * Overriden to return <tt>item()</tt>.
         *
         * @returns BuiltinTypes::item
         */
        virtual ItemType::Ptr xdtSuperType() const;

        /**
         * Overriden to return @c xs:anySimpleType.
         *
         * @returns BuiltinTypes::xsAnySimpleType
         */
        virtual SchemaType::Ptr wxsSuperType() const;

        /**
         * Overriden to return @c true, @c xs:anyAtomicType is abstract.
         *
         * @returns always @c true
         */
        virtual bool isAbstract() const;

    protected:
        friend class BuiltinTypes;
        AnyAtomicType();
    };

    /**
     * @short Implements the type @c xs:untypedAtomic.
     *
     * @ingroup Patternist_types
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class UntypedAtomicType : public BuiltinAtomicType
    {
    public:
        typedef PlainSharedPtr<UntypedAtomicType> Ptr;

        virtual AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &visitor,
                                                    const SourceLocationReflection *const reflection) const;
        virtual AtomicTypeVisitorResult::Ptr accept(const ParameterizedAtomicTypeVisitor::Ptr &visitor,
                                                    const qint16 op,
                                                    const SourceLocationReflection *const reflection) const;
        virtual QName name(const NamePool::Ptr &np) const;
        virtual QString displayName(const NamePool::Ptr &np) const;

    protected:
        friend class BuiltinTypes;
        UntypedAtomicType();
    };

    /**
     * @short Implements the type @c xs:dateTime.
     *
     * @ingroup Patternist_types
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class DateTimeType : public BuiltinAtomicType
    {
    public:
        typedef PlainSharedPtr<DateTimeType> Ptr;

        virtual AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &visitor,
                                                    const SourceLocationReflection *const reflection) const;
        virtual AtomicTypeVisitorResult::Ptr accept(const ParameterizedAtomicTypeVisitor::Ptr &visitor,
                                                    const qint16 op,
                                                    const SourceLocationReflection *const reflection) const;

        virtual QName name(const NamePool::Ptr &np) const;
        virtual QString displayName(const NamePool::Ptr &np) const;
    protected:
        friend class BuiltinTypes;
        DateTimeType();
    };

    /**
     * @short Implements the type @c xs:date.
     *
     * @ingroup Patternist_types
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class DateType : public BuiltinAtomicType
    {
    public:
        typedef PlainSharedPtr<DateType> Ptr;

        virtual AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &visitor,
                                                    const SourceLocationReflection *const reflection) const;
        virtual AtomicTypeVisitorResult::Ptr accept(const ParameterizedAtomicTypeVisitor::Ptr &visitor,
                                                    const qint16 op,
                                                    const SourceLocationReflection *const reflection) const;
        virtual QName name(const NamePool::Ptr &np) const;
        virtual QString displayName(const NamePool::Ptr &np) const;

    protected:
        friend class BuiltinTypes;
        DateType();
    };

    /**
     * @short Implements the type @c xs:time.
     *
     * @ingroup Patternist_types
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class TimeType : public BuiltinAtomicType
    {
    public:
        typedef PlainSharedPtr<TimeType> Ptr;

        virtual AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &visitor,
                                                    const SourceLocationReflection *const reflection) const;
        virtual AtomicTypeVisitorResult::Ptr accept(const ParameterizedAtomicTypeVisitor::Ptr &visitor,
                                                    const qint16 op,
                                                    const SourceLocationReflection *const reflection) const;

        virtual QName name(const NamePool::Ptr &np) const;
        virtual QString displayName(const NamePool::Ptr &np) const;

    protected:
        friend class BuiltinTypes;
        TimeType();
    };

    /**
     * @short Implements the type @c xs:duration.
     *
     * @ingroup Patternist_types
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class DurationType : public BuiltinAtomicType
    {
    public:
        typedef PlainSharedPtr<DurationType> Ptr;

        virtual AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &visitor,
                                                    const SourceLocationReflection *const reflection) const;
        virtual AtomicTypeVisitorResult::Ptr accept(const ParameterizedAtomicTypeVisitor::Ptr &visitor,
                                                    const qint16 op,
                                                    const SourceLocationReflection *const reflection) const;
        virtual QName name(const NamePool::Ptr &np) const;
        virtual QString displayName(const NamePool::Ptr &np) const;

    protected:
        friend class BuiltinTypes;
        DurationType();
    };

    /**
     * @short Implements the type @c xs:yearMonthDuration.
     *
     * @ingroup Patternist_types
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class YearMonthDurationType : public BuiltinAtomicType
    {
    public:
        typedef PlainSharedPtr<YearMonthDurationType> Ptr;

        virtual AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &visitor,
                                                    const SourceLocationReflection *const reflection) const;
        virtual AtomicTypeVisitorResult::Ptr accept(const ParameterizedAtomicTypeVisitor::Ptr &visitor,
                                                    const qint16 op,
                                                    const SourceLocationReflection *const reflection) const;
        virtual QName name(const NamePool::Ptr &np) const;
        virtual QString displayName(const NamePool::Ptr &np) const;

    protected:
        friend class BuiltinTypes;
        YearMonthDurationType();
    };

    /**
     * @short Implements the type @c xs:dayTimeDuration.
     *
     * @ingroup Patternist_types
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class DayTimeDurationType : public BuiltinAtomicType
    {
    public:
        typedef PlainSharedPtr<DayTimeDurationType> Ptr;

        virtual AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &visitor,
                                                    const SourceLocationReflection *const reflection) const;
        virtual AtomicTypeVisitorResult::Ptr accept(const ParameterizedAtomicTypeVisitor::Ptr &visitor,
                                                    const qint16 op,
                                                    const SourceLocationReflection *const reflection) const;
        virtual QName name(const NamePool::Ptr &np) const;
        virtual QString displayName(const NamePool::Ptr &np) const;

    protected:
        friend class BuiltinTypes;
        DayTimeDurationType();
    };

    /**
     * @short Implements the type @c xs:double.
     *
     * @ingroup Patternist_types
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class DoubleType : public BuiltinAtomicType
    {
    public:
        typedef PlainSharedPtr<DoubleType> Ptr;

        virtual AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &visitor,
                                                    const SourceLocationReflection *const reflection) const;
        virtual AtomicTypeVisitorResult::Ptr accept(const ParameterizedAtomicTypeVisitor::Ptr &visitor,
                                                    const qint16 op,
                                                    const SourceLocationReflection *const reflection) const;
        virtual QName name(const NamePool::Ptr &np) const;
        virtual QString displayName(const NamePool::Ptr &np) const;

    protected:
        friend class BuiltinTypes;
        DoubleType();
    };

    /**
     * @short Implements the type @c xs:float.
     *
     * @ingroup Patternist_types
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class FloatType : public BuiltinAtomicType
    {
    public:
        typedef PlainSharedPtr<FloatType> Ptr;

        virtual AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &visitor,
                                                    const SourceLocationReflection *const reflection) const;
        virtual AtomicTypeVisitorResult::Ptr accept(const ParameterizedAtomicTypeVisitor::Ptr &visitor,
                                                    const qint16 op,
                                                    const SourceLocationReflection *const reflection) const;
        virtual QName name(const NamePool::Ptr &np) const;
        virtual QString displayName(const NamePool::Ptr &np) const;

    protected:
        FloatType();
        friend class BuiltinTypes;
    };

    /**
     * @short Implements the type @c xs:decimal.
     *
     * @ingroup Patternist_types
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class DecimalType : public BuiltinAtomicType
    {
    public:
        typedef PlainSharedPtr<DecimalType> Ptr;

        virtual AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &visitor,
                                                    const SourceLocationReflection *const reflection) const;
        virtual AtomicTypeVisitorResult::Ptr accept(const ParameterizedAtomicTypeVisitor::Ptr &visitor,
                                                    const qint16 op,
                                                    const SourceLocationReflection *const reflection) const;
        virtual QName name(const NamePool::Ptr &np) const;
        virtual QString displayName(const NamePool::Ptr &np) const;

    protected:
        friend class BuiltinTypes;
        DecimalType();
    };

    /**
     * @short Implements the type @c xs:integer.
     *
     * IntegerType instances are used for representing all different xs:integer
     * types. The purpose of this is that xs:integer sub-types must use the
     * class, IntegerType, in order to use the correct behavior in call
     * dispatch situations. That is, all xs:integer sub-types must use the
     * same AtomicComparator as xs:integer itself uses, and that is achieved
     * this way.
     *
     * @ingroup Patternist_types
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class IntegerType : public BuiltinAtomicType
    {
    public:
        typedef PlainSharedPtr<IntegerType> Ptr;

        virtual AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &visitor,
                                                    const SourceLocationReflection *const reflection) const;
        virtual AtomicTypeVisitorResult::Ptr accept(const ParameterizedAtomicTypeVisitor::Ptr &visitor,
                                                    const qint16 op,
                                                    const SourceLocationReflection *const reflection) const;
        virtual QName name(const NamePool::Ptr &np) const;
        virtual QString displayName(const NamePool::Ptr &np) const;

    protected:
        friend class BuiltinTypes;
        IntegerType(const AtomicType::Ptr &parentType,
                    const AtomicCasterLocator::Ptr &casterLocator);
    };

    template<const TypeOfDerivedInteger derivedType>
    class DerivedIntegerType : public IntegerType
    {
    public:
        using IntegerType::accept;

        virtual AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &v,
                                                    const SourceLocationReflection *const r) const
        {
            qDebug() << Q_FUNC_INFO;
            return v->visit(this, r);
        }

        virtual QName name(const NamePool::Ptr &np) const
        {
            switch(derivedType)
            {
                case TypeByte:                  return np->allocateQName(StandardNamespaces::xs, QLatin1String("byte"));
                case TypeInt:                   return np->allocateQName(StandardNamespaces::xs, QLatin1String("int"));
                case TypeLong:                  return np->allocateQName(StandardNamespaces::xs, QLatin1String("long"));
                case TypeNegativeInteger:       return np->allocateQName(StandardNamespaces::xs, QLatin1String("negativeInteger"));
                case TypeNonNegativeInteger:    return np->allocateQName(StandardNamespaces::xs, QLatin1String("nonNegativeInteger"));
                case TypeNonPositiveInteger:    return np->allocateQName(StandardNamespaces::xs, QLatin1String("nonPositiveInteger"));
                case TypePositiveInteger:       return np->allocateQName(StandardNamespaces::xs, QLatin1String("positiveInteger"));
                case TypeShort:                 return np->allocateQName(StandardNamespaces::xs, QLatin1String("short"));
                case TypeUnsignedByte:          return np->allocateQName(StandardNamespaces::xs, QLatin1String("unsignedByte"));
                case TypeUnsignedInt:           return np->allocateQName(StandardNamespaces::xs, QLatin1String("unsignedInt"));
                case TypeUnsignedLong:          return np->allocateQName(StandardNamespaces::xs, QLatin1String("unsignedLong"));
                case TypeUnsignedShort:         return np->allocateQName(StandardNamespaces::xs, QLatin1String("unsignedShort"));
            }

            Q_ASSERT_X(false, "DerivedIntegerType::name()", "Invalid value in instantiation.");
            return QName();
        }

        virtual QString displayName(const NamePool::Ptr &np) const
        {
            return np->displayName(name(np));
        }

    protected:
        friend class BuiltinTypes;

        DerivedIntegerType(const AtomicType::Ptr &parentType,
                           const AtomicCasterLocator::Ptr &casterLoc) : IntegerType(parentType, casterLoc)
        {
        }

    };

    /**
     * @short Implements the type @c xs:gYearMonth.
     *
     * @ingroup Patternist_types
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class GYearMonthType : public BuiltinAtomicType
    {
    public:
        typedef PlainSharedPtr<GYearMonthType> Ptr;

        virtual AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &visitor,
                                                    const SourceLocationReflection *const reflection) const;
        virtual AtomicTypeVisitorResult::Ptr accept(const ParameterizedAtomicTypeVisitor::Ptr &visitor,
                                                    const qint16 op,
                                                    const SourceLocationReflection *const reflection) const;
        virtual QName name(const NamePool::Ptr &np) const;
        virtual QString displayName(const NamePool::Ptr &np) const;

    protected:
        friend class BuiltinTypes;
        GYearMonthType();
    };

    /**
     * @short Implements the type @c xs:gYear.
     *
     * @ingroup Patternist_types
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class GYearType : public BuiltinAtomicType
    {
    public:
        typedef PlainSharedPtr<GYearType> Ptr;

        virtual AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &visitor,
                                                    const SourceLocationReflection *const reflection) const;
        virtual AtomicTypeVisitorResult::Ptr accept(const ParameterizedAtomicTypeVisitor::Ptr &visitor,
                                                    const qint16 op,
                                                    const SourceLocationReflection *const reflection) const;
        virtual QName name(const NamePool::Ptr &np) const;
        virtual QString displayName(const NamePool::Ptr &np) const;

    protected:
        friend class BuiltinTypes;
        GYearType();
    };

    /**
     * @short Implements the type @c xs:gMonthDay.
     *
     * @ingroup Patternist_types
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class GMonthDayType : public BuiltinAtomicType
    {
    public:
        typedef PlainSharedPtr<GMonthDayType> Ptr;

        virtual AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &visitor,
                                                    const SourceLocationReflection *const reflection) const;
        virtual AtomicTypeVisitorResult::Ptr accept(const ParameterizedAtomicTypeVisitor::Ptr &visitor,
                                                    const qint16 op,
                                                    const SourceLocationReflection *const reflection) const;
        virtual QName name(const NamePool::Ptr &np) const;
        virtual QString displayName(const NamePool::Ptr &np) const;

    protected:
        friend class BuiltinTypes;
        GMonthDayType();
    };

    /**
     * @short Implements the type @c xs:gDay.
     *
     * @ingroup Patternist_types
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class GDayType : public BuiltinAtomicType
    {
    public:
        typedef PlainSharedPtr<GDayType> Ptr;

        virtual AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &visitor,
                                                    const SourceLocationReflection *const reflection) const;
        virtual AtomicTypeVisitorResult::Ptr accept(const ParameterizedAtomicTypeVisitor::Ptr &visitor,
                                                    const qint16 op,
                                                    const SourceLocationReflection *const reflection) const;
        virtual QName name(const NamePool::Ptr &np) const;
        virtual QString displayName(const NamePool::Ptr &np) const;

    protected:
        friend class BuiltinTypes;
        GDayType();
    };

    /**
     * @short Implements the type @c xs:gMonth.
     *
     * @ingroup Patternist_types
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class GMonthType : public BuiltinAtomicType
    {
    public:
        typedef PlainSharedPtr<GMonthType> Ptr;

        virtual AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &visitor,
                                                    const SourceLocationReflection *const reflection) const;
        virtual AtomicTypeVisitorResult::Ptr accept(const ParameterizedAtomicTypeVisitor::Ptr &visitor,
                                                    const qint16 op,
                                                    const SourceLocationReflection *const reflection) const;
        virtual QName name(const NamePool::Ptr &np) const;
        virtual QString displayName(const NamePool::Ptr &np) const;

    protected:
        friend class BuiltinTypes;
        GMonthType();
    };

    /**
     * @short Implements the type @c xs:boolean.
     *
     * @ingroup Patternist_types
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class BooleanType : public BuiltinAtomicType
    {
    public:
        typedef PlainSharedPtr<BooleanType> Ptr;

        virtual AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &visitor,
                                                    const SourceLocationReflection *const reflection) const;
        virtual AtomicTypeVisitorResult::Ptr accept(const ParameterizedAtomicTypeVisitor::Ptr &visitor,
                                                    const qint16 op,
                                                    const SourceLocationReflection *const reflection) const;
        virtual QName name(const NamePool::Ptr &np) const;
        virtual QString displayName(const NamePool::Ptr &np) const;

    protected:
        friend class BuiltinTypes;
        BooleanType();
    };

    /**
     * @short Implements the type @c xs:base64Binary.
     *
     * @ingroup Patternist_types
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class Base64BinaryType : public BuiltinAtomicType
    {
    public:
        typedef PlainSharedPtr<Base64BinaryType> Ptr;

        virtual AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &visitor,
                                                    const SourceLocationReflection *const reflection) const;
        virtual AtomicTypeVisitorResult::Ptr accept(const ParameterizedAtomicTypeVisitor::Ptr &visitor,
                                                    const qint16 op,
                                                    const SourceLocationReflection *const reflection) const;
        virtual QName name(const NamePool::Ptr &np) const;
        virtual QString displayName(const NamePool::Ptr &np) const;

    protected:
        friend class BuiltinTypes;
        Base64BinaryType();
    };

    /**
     * @short Implements the type @c xs:hexBinary.
     *
     * @ingroup Patternist_types
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class HexBinaryType : public BuiltinAtomicType
    {
    public:
        typedef PlainSharedPtr<HexBinaryType> Ptr;

        virtual AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &visitor,
                                                    const SourceLocationReflection *const reflection) const;
        virtual AtomicTypeVisitorResult::Ptr accept(const ParameterizedAtomicTypeVisitor::Ptr &visitor,
                                                    const qint16 op,
                                                    const SourceLocationReflection *const reflection) const;
        virtual QName name(const NamePool::Ptr &np) const;
        virtual QString displayName(const NamePool::Ptr &np) const;

    protected:
        friend class BuiltinTypes;
        HexBinaryType();
    };

    /**
     * @short Implements the type @c xs:anyURI.
     *
     * @ingroup Patternist_types
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class AnyURIType : public BuiltinAtomicType
    {
    public:
        typedef PlainSharedPtr<AnyURIType> Ptr;

        virtual AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &visitor,
                                                    const SourceLocationReflection *const reflection) const;
        virtual AtomicTypeVisitorResult::Ptr accept(const ParameterizedAtomicTypeVisitor::Ptr &visitor,
                                                    const qint16 op,
                                                    const SourceLocationReflection *const reflection) const;
        virtual QName name(const NamePool::Ptr &np) const;
        virtual QString displayName(const NamePool::Ptr &np) const;

    protected:
        friend class BuiltinTypes;
        AnyURIType();
    };

    /**
     * @short Implements the type @c xs:QName.
     *
     * @ingroup Patternist_types
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class QNameType : public BuiltinAtomicType
    {
    public:
        typedef PlainSharedPtr<QNameType> Ptr;

        virtual AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &visitor,
                                                    const SourceLocationReflection *const reflection) const;
        virtual AtomicTypeVisitorResult::Ptr accept(const ParameterizedAtomicTypeVisitor::Ptr &visitor,
                                                    const qint16 op,
                                                    const SourceLocationReflection *const reflection) const;
        virtual QName name(const NamePool::Ptr &np) const;
        virtual QString displayName(const NamePool::Ptr &np) const;

    protected:
        friend class BuiltinTypes;
        QNameType();
    };

    /**
     * Represents the xs:string type and all derived types of
     * xs:string, such as xs:token.
     *
     * StringType instances are used for representing all different string
     * types. The purpose of this is that xs:string sub-types must use the
     * class, StringType, in order to use the correct behavior in call
     * dispatch situations. That is, all xs:string sub-types must use the
     * same AtomicComparator as xs:string itself uses, and that is achieved
     * this way.
     *
     * @ingroup Patternist_types
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class StringType : public BuiltinAtomicType
    {
    public:
        typedef PlainSharedPtr<StringType> Ptr;

        virtual AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &visitor,
                                                    const SourceLocationReflection *const reflection) const;
        virtual AtomicTypeVisitorResult::Ptr accept(const ParameterizedAtomicTypeVisitor::Ptr &visitor,
                                                    const qint16 op,
                                                    const SourceLocationReflection *const reflection) const;
        virtual QName name(const NamePool::Ptr &np) const;
        virtual QString displayName(const NamePool::Ptr &np) const;

    protected:
        friend class BuiltinTypes;
        StringType(const AtomicType::Ptr &parentType,
                   const AtomicCasterLocator::Ptr &casterLoc);
    };

    template<const TypeOfDerivedString derivedType>
    class DerivedStringType : public StringType
    {
    public:
        using StringType::accept;

        virtual AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &v,
                                                    const SourceLocationReflection *const r) const
        {
            qDebug() << Q_FUNC_INFO;
            return v->visit(this, r);
        }

        virtual QName name(const NamePool::Ptr &np) const
        {
            switch(derivedType)
            {
                case TypeString:            return np->allocateQName(StandardNamespaces::xs, QLatin1String("string"));
                case TypeNormalizedString:  return np->allocateQName(StandardNamespaces::xs, QLatin1String("normalizedString"));
                case TypeToken:             return np->allocateQName(StandardNamespaces::xs, QLatin1String("token"));
                case TypeLanguage:          return np->allocateQName(StandardNamespaces::xs, QLatin1String("language"));
                case TypeNMTOKEN:           return np->allocateQName(StandardNamespaces::xs, QLatin1String("NMTOKEN"));
                case TypeName:              return np->allocateQName(StandardNamespaces::xs, QLatin1String("Name"));
                case TypeNCName:            return np->allocateQName(StandardNamespaces::xs, QLatin1String("NCName"));
                case TypeID:                return np->allocateQName(StandardNamespaces::xs, QLatin1String("ID"));
                case TypeIDREF:             return np->allocateQName(StandardNamespaces::xs, QLatin1String("IDREF"));
                case TypeENTITY:            return np->allocateQName(StandardNamespaces::xs, QLatin1String("ENTITY"));
            }

            Q_ASSERT_X(false, "DerivedStringType::name()", "Invalid value in instantiation.");
            return QName();
        }

        virtual QString displayName(const NamePool::Ptr &np) const
        {
            return np->displayName(name(np));
        }

    protected:
        friend class BuiltinTypes;

        DerivedStringType(const AtomicType::Ptr &parentType,
                          const AtomicCasterLocator::Ptr &casterLoc) : StringType(parentType, casterLoc)
        {
        }

    };

    /**
     * @short Implements the type @c xs:NOTATION.
     *
     * @ingroup Patternist_types
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class NOTATIONType : public BuiltinAtomicType
    {
    public:
        typedef PlainSharedPtr<NOTATIONType> Ptr;

        virtual AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &visitor,
                                                    const SourceLocationReflection *const reflection) const;
        virtual AtomicTypeVisitorResult::Ptr accept(const ParameterizedAtomicTypeVisitor::Ptr &visitor,
                                                    const qint16 op,
                                                    const SourceLocationReflection *const reflection) const;
        virtual QName name(const NamePool::Ptr &np) const;
        virtual QString displayName(const NamePool::Ptr &np) const;

        /**
         * Overriden to return @c true, xs:NOTATION is abstract.
         *
         * @returns always @c true
         */
        virtual bool isAbstract() const;

    protected:
        friend class BuiltinTypes;
        NOTATIONType();
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

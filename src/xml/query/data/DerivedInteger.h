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

#ifndef Patternist_DerivedInteger_H
#define Patternist_DerivedInteger_H

#include "BuiltinTypes.h"
#include "Debug.h"
#include "Integer.h"
#include "PatternistLocale.h"
#include "ValidationError.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @relates DerivedInteger
     */
    enum DerivedIntegerLimitsUsage
    {
        None            = 1,
        LimitUpwards    = 2,
        LimitDownwards  = 4,
        LimitBoth       = LimitUpwards | LimitDownwards
    };

    enum
    {
        IgnorableSignedValue = 0,
        IgnorableUnsignedValue = 0
    };

    template<const TypeOfDerivedInteger> class DerivedInteger;

    template<const TypeOfDerivedInteger> class DerivedIntegerDetails;

    template<>
    class DerivedIntegerDetails<TypeByte>
    {
    private:
        friend class DerivedInteger<TypeByte>;
        typedef qint8                           StorageType;
        typedef xsInteger                       TemporaryStorageType;
        static const StorageType                maxInclusive = 127;
        static const StorageType                minInclusive = -128;
        static const DerivedIntegerLimitsUsage  limitsUsage = LimitBoth;

        /**
         * Disable the default constructor.
         */
        DerivedIntegerDetails() {}

        Q_DISABLE_COPY(DerivedIntegerDetails)
    };

    template<>
    class DerivedIntegerDetails<TypeInt>
    {
    private:
        friend class DerivedInteger<TypeInt>;
        typedef qint32                          StorageType;
        typedef xsInteger                       TemporaryStorageType;
        static const StorageType                maxInclusive = Q_INT64_C(2147483647);
        static const StorageType                minInclusive = Q_INT64_C(-2147483648);
        static const DerivedIntegerLimitsUsage  limitsUsage = LimitBoth;

        /**
         * Disable the default constructor.
         */
        DerivedIntegerDetails() {}

        Q_DISABLE_COPY(DerivedIntegerDetails)
    };

    template<>
    class DerivedIntegerDetails<TypeLong>
    {
    private:
        friend class DerivedInteger<TypeLong>;
        typedef qint64                          StorageType;
        typedef StorageType                     TemporaryStorageType;
        static const StorageType                maxInclusive = Q_INT64_C(9223372036854775807);
        static const StorageType                minInclusive = Q_UINT64_C(-9223372036854775808);
        static const DerivedIntegerLimitsUsage  limitsUsage = LimitBoth;

        /**
         * Disable the default constructor.
         */
        DerivedIntegerDetails() {}

        Q_DISABLE_COPY(DerivedIntegerDetails)
    };

    template<>
    class DerivedIntegerDetails<TypeNegativeInteger>
    {
    private:
        friend class DerivedInteger<TypeNegativeInteger>;
        typedef xsInteger                       StorageType;
        typedef StorageType                     TemporaryStorageType;
        static const StorageType                maxInclusive = -1;
        static const StorageType                minInclusive = IgnorableSignedValue;
        static const DerivedIntegerLimitsUsage  limitsUsage = LimitUpwards;

        /**
         * Disable the default constructor.
         */
        DerivedIntegerDetails() {}

        Q_DISABLE_COPY(DerivedIntegerDetails)
    };

    template<>
    class DerivedIntegerDetails<TypeNonNegativeInteger>
    {
    private:
        friend class DerivedInteger<TypeNonNegativeInteger>;
        typedef xsInteger                       StorageType;
        typedef StorageType                     TemporaryStorageType;
        static const StorageType                maxInclusive = IgnorableSignedValue;
        static const StorageType                minInclusive = 0;
        static const DerivedIntegerLimitsUsage  limitsUsage = LimitDownwards;

        /**
         * Disable the default constructor.
         */
        DerivedIntegerDetails() {}

        Q_DISABLE_COPY(DerivedIntegerDetails)
    };

    template<>
    class DerivedIntegerDetails<TypeNonPositiveInteger>
    {
    private:
        friend class DerivedInteger<TypeNonPositiveInteger>;
        typedef xsInteger                       StorageType;
        typedef StorageType                     TemporaryStorageType;
        static const StorageType                maxInclusive = 0;
        static const StorageType                minInclusive = IgnorableSignedValue;
        static const DerivedIntegerLimitsUsage  limitsUsage = LimitUpwards;

        /**
         * Disable the default constructor.
         */
        DerivedIntegerDetails() {}

        Q_DISABLE_COPY(DerivedIntegerDetails)
    };

    template<>
    class DerivedIntegerDetails<TypePositiveInteger>
    {
    private:
        friend class DerivedInteger<TypePositiveInteger>;
        typedef xsInteger                       StorageType;
        typedef StorageType                     TemporaryStorageType;
        static const StorageType                maxInclusive = IgnorableSignedValue;
        static const StorageType                minInclusive = 1;
        static const DerivedIntegerLimitsUsage  limitsUsage = LimitDownwards;

        /**
         * Disable the default constructor.
         */
        DerivedIntegerDetails() {}

        Q_DISABLE_COPY(DerivedIntegerDetails)
    };

    template<>
    class DerivedIntegerDetails<TypeShort>
    {
    private:
        friend class DerivedInteger<TypeShort>;
        typedef qint16                          StorageType;
        typedef xsInteger                       TemporaryStorageType;
        static const StorageType                maxInclusive = 32767;
        static const StorageType                minInclusive = -32768;
        static const DerivedIntegerLimitsUsage  limitsUsage = LimitBoth;

        /**
         * Disable the default constructor.
         */
        DerivedIntegerDetails() {}

        Q_DISABLE_COPY(DerivedIntegerDetails)
    };

    template<>
    class DerivedIntegerDetails<TypeUnsignedByte>
    {
    private:
        friend class DerivedInteger<TypeUnsignedByte>;
        typedef quint8                          StorageType;
        typedef qint64                          TemporaryStorageType;
        static const StorageType                maxInclusive = 255;
        static const StorageType                minInclusive = 0;
        static const DerivedIntegerLimitsUsage  limitsUsage = LimitBoth;

        /**
         * Disable the default constructor.
         */
        DerivedIntegerDetails() {}

        Q_DISABLE_COPY(DerivedIntegerDetails)
    };

    template<>
    class DerivedIntegerDetails<TypeUnsignedInt>
    {
    private:
        friend class DerivedInteger<TypeUnsignedInt>;
        typedef quint32                         StorageType;
        typedef qint64                          TemporaryStorageType;
        static const StorageType                maxInclusive = Q_UINT64_C(4294967295);
        static const StorageType                minInclusive = 0;
        static const DerivedIntegerLimitsUsage  limitsUsage = LimitBoth;

        /**
         * Disable the default constructor.
         */
        DerivedIntegerDetails() {}

        Q_DISABLE_COPY(DerivedIntegerDetails)
    };

    template<>
    class DerivedIntegerDetails<TypeUnsignedLong>
    {
    private:
        friend class DerivedInteger<TypeUnsignedLong>;
        typedef quint64                         StorageType;
        typedef StorageType                     TemporaryStorageType;
        static const StorageType                maxInclusive = Q_UINT64_C(18446744073709551615);
        static const StorageType                minInclusive = 0;
        static const DerivedIntegerLimitsUsage  limitsUsage = LimitBoth;

        /**
         * Disable the default constructor.
         */
        DerivedIntegerDetails() {}

        Q_DISABLE_COPY(DerivedIntegerDetails)
    };

    template<>
    class DerivedIntegerDetails<TypeUnsignedShort>
    {
    private:
        friend class DerivedInteger<TypeUnsignedShort>;
        typedef quint16                         StorageType;
        typedef qint64                          TemporaryStorageType;
        static const StorageType                maxInclusive = 65535;
        static const StorageType                minInclusive = 0;
        static const DerivedIntegerLimitsUsage  limitsUsage = LimitBoth;

        /**
         * Disable the default constructor.
         */
        DerivedIntegerDetails() {}

        Q_DISABLE_COPY(DerivedIntegerDetails)
    };

    /**
     * @short Represents instances of derived @c xs:integer types, such as @c
     * xs:byte.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_xdm
     */
    template<const TypeOfDerivedInteger DerivedType>
    class DerivedInteger : public Numeric
    {
    private:
        typedef typename DerivedIntegerDetails<DerivedType>::StorageType StorageType;
        typedef typename DerivedIntegerDetails<DerivedType>::TemporaryStorageType TemporaryStorageType;

        static const StorageType                maxInclusive        = DerivedIntegerDetails<DerivedType>::maxInclusive;
        static const StorageType                minInclusive        = DerivedIntegerDetails<DerivedType>::minInclusive;
        static const DerivedIntegerLimitsUsage  limitsUsage         = DerivedIntegerDetails<DerivedType>::limitsUsage;

        const StorageType m_value;

        inline DerivedInteger(const StorageType num) : m_value(num)
        {
            qDebug() << "DerivedInteger ctor: " << num;
        }

    public:

        static ItemType::Ptr itemType()
        {
            switch(DerivedType)
            {
                case TypeByte:                  return BuiltinTypes::xsByte;
                case TypeInt:                   return BuiltinTypes::xsInt;
                case TypeLong:                  return BuiltinTypes::xsLong;
                case TypeNegativeInteger:       return BuiltinTypes::xsNegativeInteger;
                case TypeNonNegativeInteger:    return BuiltinTypes::xsNonNegativeInteger;
                case TypeNonPositiveInteger:    return BuiltinTypes::xsNonPositiveInteger;
                case TypePositiveInteger:       return BuiltinTypes::xsPositiveInteger;
                case TypeShort:                 return BuiltinTypes::xsShort;
                case TypeUnsignedByte:          return BuiltinTypes::xsUnsignedByte;
                case TypeUnsignedInt:           return BuiltinTypes::xsUnsignedInt;
                case TypeUnsignedLong:          return BuiltinTypes::xsUnsignedLong;
                case TypeUnsignedShort:         return BuiltinTypes::xsUnsignedShort;
            }

            Q_ASSERT(false);
            return ItemType::Ptr();
        }

        static AtomicValue::Ptr fromValue(const NamePool::Ptr &np, const TemporaryStorageType num)
        {
            if((limitsUsage & LimitUpwards) &&
               num > maxInclusive)
            {
                return ValidationError::createError(tr("For the type %1 the value %2 is too large since the maximum "
                                                       "value is %3, inclusive.")
                                                    .arg(formatType(np, itemType()))
                                                    .arg(formatData(static_cast<xsInteger>(num)))
                                                    .arg(formatData(static_cast<xsInteger>(maxInclusive))));
            }
            /* The test below generates the warning "warning: comparison of unsigned expression < 0 is always false" with gcc
             * when the class is instantiated with TypeUnsignedLong. It is a false positive warning, but it's tricky to
             * remove, as I see it. */
            else if((limitsUsage & LimitDownwards) &&
                    num < minInclusive)
            {
                return ValidationError::createError(tr("For the type %1 the value %2 "
                                                       "is too large since the maximum "
                                                       "value is %3, inclusive.")
                                                    .arg(formatType(np, itemType()))
                                                    .arg(formatData(static_cast<xsInteger>(num)))
                                                    .arg(formatData(static_cast<xsInteger>(minInclusive))));
            }
            else
                return AtomicValue::Ptr(new DerivedInteger(num));
        }

        /**
         * Constructs an instance from the lexical
         * representation @p strNumeric.
         */
        static AtomicValue::Ptr fromLexical(const NamePool::Ptr &np, const QString &strNumeric)
        {
            bool conversionOk = false;
            TemporaryStorageType num;

            /* Depending on the type, we need to call different conversion
             * functions on QString. */
            switch(DerivedType)
            {
                case TypeUnsignedLong:
                {
                    /* Qt decides to flag '-' as invalid, so remove it before. */
                    if(strNumeric.contains(QLatin1Char('-')))
                    {
                        num = QString(strNumeric).remove(QLatin1Char('-')).toULongLong(&conversionOk);

                        if(num != 0)
                            conversionOk = false;
                    }
                    else
                        num = strNumeric.toULongLong(&conversionOk);

                    break;
                }
                default:
                {
                    num = strNumeric.toLongLong(&conversionOk);
                    break;
                }
            }

            if(conversionOk)
                return fromValue(np, num);
            else
                return ValidationError::createError();
        }

        /**
         * Determines the Effective %Boolean Value of this number.
         *
         * @returns @c false if the number is 0, otherwise @c true.
         */
        bool evaluateEBV(const PlainSharedPtr<DynamicContext> &) const
        {
            return m_value != 0;
        }

        virtual QString stringValue() const
        {
            return QString::number(m_value);
        }

        virtual ItemType::Ptr type() const
        {
            return itemType();
        }

        virtual xsDouble toDouble() const
        {
            return static_cast<xsDouble>(m_value);
        }

        virtual xsInteger toInteger() const
        {
            return m_value;
        }

        virtual xsFloat toFloat() const
        {
            return static_cast<xsFloat>(m_value);
        }

        virtual xsDecimal toDecimal() const
        {
            return static_cast<xsDecimal>(m_value);
        }

        virtual Numeric::Ptr round() const
        {
            /* xs:integerS never have a mantissa. */
            //return Integer::fromValue(m_value);
            //return Numeric::Ptr();
            return Numeric::Ptr(static_cast<Numeric *>(Integer::fromValue(m_value).asAtomicValue()));
        }

        virtual Numeric::Ptr roundHalfToEven(const xsInteger) const
        {
            //return Numeric::Ptr();
            return Numeric::Ptr(static_cast<Numeric *>(Integer::fromValue(m_value).asAtomicValue()));
        }

        virtual Numeric::Ptr floor() const
        {
            return Numeric::Ptr(static_cast<Numeric *>(Integer::fromValue(m_value).asAtomicValue()));
        }

        virtual Numeric::Ptr ceiling() const
        {
            //return Numeric::Ptr(Integer::fromValue(m_value));
            return Numeric::Ptr(static_cast<Numeric *>(Integer::fromValue(m_value).asAtomicValue()));
        }

        virtual Numeric::Ptr abs() const
        {
            /* We unconditionally create an Integer even if we're a positive
             * value, because one part of this is the type change to
             * xs:integer.
             *
             * We've manually inlined qAbs() and invoke xsInteger's
             * constructor. The reason being that we other gets truncation down
             * to StorageType. See for instance absint1args-1. */
            //return Numeric::Ptr(Integer::fromValue(m_value >= 0 ? xsInteger(m_value) : -xsInteger(m_value)).as<Numeric>());
            return Numeric::Ptr(static_cast<Numeric *>(Integer::fromValue(m_value >= 0 ? xsInteger(m_value) : -xsInteger(m_value)).asAtomicValue()));
        }

        /**
         * @returns always @c false, @c xs:DerivedInteger doesn't have
         * not-a-number in its value space.
         */
        virtual bool isNaN() const
        {
            return false;
        }

        /**
         * @returns always @c false, @c xs:DerivedInteger doesn't have
         * infinity in its value space.
         */
        virtual bool isInf() const
        {
            return false;
        }
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

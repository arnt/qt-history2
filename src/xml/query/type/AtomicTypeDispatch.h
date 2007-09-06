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

#ifndef Patternist_AtomicTypeDispatch_H
#define Patternist_AtomicTypeDispatch_H

#include <QSharedData>

#include "PlainSharedPtr.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    class AnyAtomicType;
    class AnyURIType;
    class Base64BinaryType;
    class BooleanType;
    class DateTimeType;
    class DateType;
    class DayTimeDurationType;
    class DecimalType;
    class DoubleType;
    class DurationType;
    class FloatType;
    class GDayType;
    class GMonthDayType;
    class GMonthType;
    class GYearMonthType;
    class GYearType;
    class HexBinaryType;
    class IntegerType;
    class NOTATIONType;
    class QNameType;
    class SourceLocationReflection;
    class StringType;
    class SchemaTimeType;
    class UntypedAtomicType;
    class YearMonthDurationType;

    enum TypeOfDerivedInteger
    {
        TypeByte,
        TypeInt,
        TypeLong,
        TypeNegativeInteger,
        TypeNonNegativeInteger,
        TypeNonPositiveInteger,
        TypePositiveInteger,
        TypeShort,
        TypeUnsignedByte,
        TypeUnsignedInt,
        TypeUnsignedLong,
        TypeUnsignedShort
    };

    template<const TypeOfDerivedInteger> class DerivedIntegerType;

    enum TypeOfDerivedString
    {
        TypeString,
        TypeNormalizedString,
        TypeToken,
        TypeLanguage,
        TypeNMTOKEN,
        TypeName,
        TypeNCName,
        TypeID,
        TypeIDREF,
        TypeENTITY
    };

    template<const TypeOfDerivedString> class DerivedStringType;

    /**
     * @todo Documentation's missing:
     * - Add link to wikipedia's "multiple dispatch" and "visitor" page.
     * - Add link to http://www.eptacom.net/pubblicazioni/pub_eng/mdisp.html
     *
     * @defgroup Patternist_types_dispatch Atomic Type Dispatching
     */

    /**
     * @todo Docs missing
     *
     * @ingroup Patternist_types_dispatch
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class AtomicTypeVisitorResult : public QSharedData
    {
    public:
        typedef PlainSharedPtr<AtomicTypeVisitorResult> Ptr;
        AtomicTypeVisitorResult() {}
        virtual ~AtomicTypeVisitorResult() {}
    };

    /**
     * @todo Docs missing
     *
     * @see ParameterizedAtomicTypeVisitor
     * @ingroup Patternist_types_dispatch
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class AtomicTypeVisitor : public QSharedData
    {
    public:
        typedef PlainSharedPtr<AtomicTypeVisitor> Ptr;
        virtual ~AtomicTypeVisitor() {}

        virtual AtomicTypeVisitorResult::Ptr visit(const AnyAtomicType *,
                                                   const SourceLocationReflection *const reflection) const = 0;
        virtual AtomicTypeVisitorResult::Ptr visit(const AnyURIType *,
                                                   const SourceLocationReflection *const reflection) const = 0;
        virtual AtomicTypeVisitorResult::Ptr visit(const Base64BinaryType *,
                                                   const SourceLocationReflection *const reflection) const = 0;
        virtual AtomicTypeVisitorResult::Ptr visit(const BooleanType *,
                                                   const SourceLocationReflection *const reflection) const = 0;
        virtual AtomicTypeVisitorResult::Ptr visit(const DateTimeType *,
                                                   const SourceLocationReflection *const reflection) const = 0;
        virtual AtomicTypeVisitorResult::Ptr visit(const DateType *,
                                                   const SourceLocationReflection *const reflection) const = 0;
        virtual AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *,
                                                   const SourceLocationReflection *const reflection) const = 0;
        virtual AtomicTypeVisitorResult::Ptr visit(const DecimalType *,
                                                   const SourceLocationReflection *const reflection) const = 0;
        virtual AtomicTypeVisitorResult::Ptr visit(const DoubleType *,
                                                   const SourceLocationReflection *const reflection) const = 0;
        virtual AtomicTypeVisitorResult::Ptr visit(const DurationType *,
                                                   const SourceLocationReflection *const reflection) const = 0;
        virtual AtomicTypeVisitorResult::Ptr visit(const FloatType *,
                                                   const SourceLocationReflection *const reflection) const = 0;
        virtual AtomicTypeVisitorResult::Ptr visit(const GDayType *,
                                                   const SourceLocationReflection *const reflection) const = 0;
        virtual AtomicTypeVisitorResult::Ptr visit(const GMonthDayType *,
                                                   const SourceLocationReflection *const reflection) const = 0;
        virtual AtomicTypeVisitorResult::Ptr visit(const GMonthType *,
                                                   const SourceLocationReflection *const reflection) const = 0;
        virtual AtomicTypeVisitorResult::Ptr visit(const GYearMonthType *,
                                                   const SourceLocationReflection *const reflection) const = 0;
        virtual AtomicTypeVisitorResult::Ptr visit(const GYearType *,
                                                   const SourceLocationReflection *const reflection) const = 0;
        virtual AtomicTypeVisitorResult::Ptr visit(const HexBinaryType *,
                                                   const SourceLocationReflection *const reflection) const = 0;
        virtual AtomicTypeVisitorResult::Ptr visit(const IntegerType *,
                                                   const SourceLocationReflection *const reflection) const = 0;
        virtual AtomicTypeVisitorResult::Ptr visit(const NOTATIONType *,
                                                   const SourceLocationReflection *const reflection) const = 0;
        virtual AtomicTypeVisitorResult::Ptr visit(const QNameType *,
                                                   const SourceLocationReflection *const reflection) const = 0;
        virtual AtomicTypeVisitorResult::Ptr visit(const StringType *,
                                                   const SourceLocationReflection *const reflection) const = 0;
        virtual AtomicTypeVisitorResult::Ptr visit(const SchemaTimeType *,
                                                   const SourceLocationReflection *const reflection) const = 0;
        virtual AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
                                                   const SourceLocationReflection *const reflection) const = 0;
        virtual AtomicTypeVisitorResult::Ptr visit(const YearMonthDurationType *,
                                                   const SourceLocationReflection *const reflection) const = 0;
    };

    /**
     * @todo Docs missing
     *
     * @see AtomicTypeVisitor
     * @ingroup Patternist_types_dispatch
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class ParameterizedAtomicTypeVisitor : public QSharedData
    {
    public:
        typedef PlainSharedPtr<ParameterizedAtomicTypeVisitor> Ptr;
        virtual ~ParameterizedAtomicTypeVisitor() {}

        virtual AtomicTypeVisitorResult::Ptr visit(const AnyAtomicType *, const qint16 param,
                                                   const SourceLocationReflection *const reflection) const = 0;
        virtual AtomicTypeVisitorResult::Ptr visit(const AnyURIType *, const qint16 param,
                                                   const SourceLocationReflection *const reflection) const = 0;
        virtual AtomicTypeVisitorResult::Ptr visit(const Base64BinaryType *, const qint16 param,
                                                   const SourceLocationReflection *const reflection) const = 0;
        virtual AtomicTypeVisitorResult::Ptr visit(const BooleanType *, const qint16 param,
                                                   const SourceLocationReflection *const reflection) const = 0;
        virtual AtomicTypeVisitorResult::Ptr visit(const DateTimeType *, const qint16 param,
                                                   const SourceLocationReflection *const reflection) const = 0;
        virtual AtomicTypeVisitorResult::Ptr visit(const DateType *, const qint16 param,
                                                   const SourceLocationReflection *const reflection) const = 0;
        virtual AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *, const qint16 param,
                                                   const SourceLocationReflection *const reflection) const = 0;
        virtual AtomicTypeVisitorResult::Ptr visit(const DecimalType *, const qint16 param,
                                                   const SourceLocationReflection *const reflection) const = 0;
        virtual AtomicTypeVisitorResult::Ptr visit(const DoubleType *, const qint16 param,
                                                   const SourceLocationReflection *const reflection) const = 0;
        virtual AtomicTypeVisitorResult::Ptr visit(const DurationType *, const qint16 param,
                                                   const SourceLocationReflection *const reflection) const = 0;
        virtual AtomicTypeVisitorResult::Ptr visit(const FloatType *, const qint16 param,
                                                   const SourceLocationReflection *const reflection) const = 0;
        virtual AtomicTypeVisitorResult::Ptr visit(const GDayType *, const qint16 param,
                                                   const SourceLocationReflection *const reflection) const = 0;
        virtual AtomicTypeVisitorResult::Ptr visit(const GMonthDayType *, const qint16 param,
                                                   const SourceLocationReflection *const reflection) const = 0;
        virtual AtomicTypeVisitorResult::Ptr visit(const GMonthType *, const qint16 param,
                                                   const SourceLocationReflection *const reflection) const = 0;
        virtual AtomicTypeVisitorResult::Ptr visit(const GYearMonthType *, const qint16 param,
                                                   const SourceLocationReflection *const reflection) const = 0;
        virtual AtomicTypeVisitorResult::Ptr visit(const GYearType *, const qint16 param,
                                                   const SourceLocationReflection *const reflection) const = 0;
        virtual AtomicTypeVisitorResult::Ptr visit(const HexBinaryType *, const qint16 param,
                                                   const SourceLocationReflection *const reflection) const = 0;
        virtual AtomicTypeVisitorResult::Ptr visit(const IntegerType *, const qint16 param,
                                                   const SourceLocationReflection *const reflection) const = 0;
        virtual AtomicTypeVisitorResult::Ptr visit(const NOTATIONType *, const qint16 param,
                                                   const SourceLocationReflection *const reflection) const = 0;
        virtual AtomicTypeVisitorResult::Ptr visit(const QNameType *, const qint16 param,
                                                   const SourceLocationReflection *const reflection) const = 0;
        virtual AtomicTypeVisitorResult::Ptr visit(const StringType *, const qint16 param,
                                                   const SourceLocationReflection *const reflection) const = 0;
        virtual AtomicTypeVisitorResult::Ptr visit(const SchemaTimeType *, const qint16 param,
                                                   const SourceLocationReflection *const reflection) const = 0;
        virtual AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *, const qint16 param,
                                                   const SourceLocationReflection *const reflection) const = 0;
        virtual AtomicTypeVisitorResult::Ptr visit(const YearMonthDurationType *, const qint16 param,
                                                   const SourceLocationReflection *const reflection) const = 0;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

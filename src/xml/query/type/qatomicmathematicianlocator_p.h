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

#ifndef Patternist_AtomicTypeVisitorResultLocator_H
#define Patternist_AtomicTypeVisitorResultLocator_H

#include "qatomictypedispatch_p.h"

QT_BEGIN_HEADER 

QT_BEGIN_NAMESPACE

namespace Patternist
{
    /**
     * @todo Docs missing
     *
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class AtomicMathematicianLocator : public ParameterizedAtomicTypeVisitor
    {
    public:
        typedef PlainSharedPtr<AtomicMathematicianLocator> Ptr;

        inline AtomicMathematicianLocator()
        {
        }

        virtual ~AtomicMathematicianLocator();

        virtual AtomicTypeVisitorResult::Ptr visit(const AnyAtomicType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const reflection) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const AnyURIType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const reflection) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const Base64BinaryType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const reflection) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const BooleanType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const reflection) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DateTimeType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const reflection) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DateType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const reflection) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const reflection) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DecimalType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const reflection) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DoubleType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const reflection) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DurationType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const reflection) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const FloatType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const reflection) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const GDayType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const reflection) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const GMonthDayType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const reflection) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const GMonthType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const reflection) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const GYearMonthType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const reflection) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const GYearType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const reflection) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const HexBinaryType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const reflection) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const IntegerType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const reflection) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const NOTATIONType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const reflection) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const QNameType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const reflection) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const StringType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const reflection) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const SchemaTimeType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const reflection) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const reflection) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const YearMonthDurationType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const reflection) const;
    };
}

QT_END_NAMESPACE

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

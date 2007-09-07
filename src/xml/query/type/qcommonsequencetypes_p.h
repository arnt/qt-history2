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

#ifndef Patternist_CommonSequenceTypes_H
#define Patternist_CommonSequenceTypes_H

#include "qemptysequencetype_p.h"
#include "qnonetype_p.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Provides access to singleton instances of SequenceType sub-classes.
     *
     * @ingroup Patternist_types
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class CommonSequenceTypes
    {
    public:
        /**
         * <tt>xs:anyAtomicType?</tt>
         */
        static const SequenceType::Ptr ZeroOrOneAtomicType;

        /**
         * <tt>xs:anyAtomicType</tt>
         */
        static const SequenceType::Ptr ExactlyOneAtomicType;

        /**
         * <tt>xs:anyAtomicType*</tt>
         */
        static const SequenceType::Ptr ZeroOrMoreAtomicTypes;

        /**
         * <tt>item()</tt>
         */
        static const SequenceType::Ptr ExactlyOneItem;

        /**
         * <tt>item()*</tt>
         */
        static const SequenceType::Ptr ZeroOrMoreItems;

        /**
         * <tt>item()?</tt>
         */
        static const SequenceType::Ptr ZeroOrOneItem;

        /**
         * <tt>item()+</tt>
         */
        static const SequenceType::Ptr OneOrMoreItems;

        /**
         * The empty sequence, <tt>empty-sequence()</tt>.
         */
        static const EmptySequenceType::Ptr Empty;

        /**
         * The special type @c none. Used for the function <tt>fn:error()</tt>, for example.
         */
        static const NoneType::Ptr None;

        /**
         * <tt>xs:anyURI</tt>
         */
        static const SequenceType::Ptr ExactlyOneAnyURI;

        /**
         * <tt>xs:boolean</tt>
         */
        static const SequenceType::Ptr ExactlyOneBoolean;

        /**
         * <tt>xs:boolean?</tt>
         */
        static const SequenceType::Ptr ZeroOrOneBoolean;

        /**
         * <tt>xs:untypedAtomic</tt>
         */
        static const SequenceType::Ptr ExactlyOneUntypedAtomic;

        /**
         * <tt>xs:integer</tt>
         */
        static const SequenceType::Ptr ExactlyOneInteger;

        /**
         * <tt>xs:integer?</tt>
         */
        static const SequenceType::Ptr ZeroOrOneInteger;

        /**
         * <tt>xs:decimal?</tt>
         */
        static const SequenceType::Ptr ZeroOrOneDecimal;

        /**
         * <tt>xs:integer*</tt>
         */
        static const SequenceType::Ptr ZeroOrMoreIntegers;

        /**
         * <tt>xs:double</tt>
         */
        static const SequenceType::Ptr ExactlyOneDouble;

        /**
         * <tt>xs:decimal</tt>
         */
        static const SequenceType::Ptr ExactlyOneDecimal;

        /**
         * <tt>xs:float</tt>
         */
        static const SequenceType::Ptr ExactlyOneFloat;

        /**
         * <tt>xs:QName</tt>
         */
        static const SequenceType::Ptr ExactlyOneQName;

        /**
         * <tt>xs:string</tt>
         */
        static const SequenceType::Ptr ExactlyOneString;

        /**
         * <tt>xs:string?</tt>
         */
        static const SequenceType::Ptr ZeroOrOneString;

        /**
         * <tt>xs:string*</tt>
         */
        static const SequenceType::Ptr ZeroOrMoreStrings;

        /**
         * <tt>xs:NCName*</tt>
         */
        static const SequenceType::Ptr ZeroOrOneNCName;

        /**
         * <tt>xs:QName?</tt>
         */
        static const SequenceType::Ptr ZeroOrOneQName;

        /**
         * The artificial type in XPath 2.0 that covers @c xs:double, @c xs:float,
         * @c xs:decimal, with cardinality zero or one.
         *
         * @see <a href="http://www.w3.org/TR/xpath20/#dt-numeric">XML Path Language
         * (XPath) 2.0, definition for Numeric</a>
         * @see <a href="http://www.w3.org/TR/xpath-functions/#func-signatures">XQuery 1.0
         * and XPath 2.0 Functions and Operators, 1.3 Function Signatures and Descriptions</a>
         * @see BuiltinTypes::numeric
         */
        static const SequenceType::Ptr ZeroOrOneNumeric;

        /**
         * @c numeric
         */
        static const SequenceType::Ptr ExactlyOneNumeric;

        /**
         * <tt>node()?</tt>
         */
        static const SequenceType::Ptr ZeroOrOneNode;

        /**
         * <tt>node()</tt>
         */
        static const SequenceType::Ptr ExactlyOneNode;

        /**
         * <tt>node()*</tt>
         */
        static const SequenceType::Ptr ZeroOrMoreNodes;

        /**
         * <tt>element()</tt>
         */
        static const SequenceType::Ptr ExactlyOneElement;

        /**
         * <tt>processing-instruction()</tt>
         */
        static const SequenceType::Ptr ExactlyOneProcessingInstruction;

        /**
         * <tt>attribute()</tt>
         */
        static const SequenceType::Ptr ExactlyOneAttribute;

        /**
         * <tt>text()</tt>
         */
        static const SequenceType::Ptr ExactlyOneTextNode;

        /**
         * <tt>text()?</tt>
         */
        static const SequenceType::Ptr ZeroOrOneTextNode;

        /**
         * <tt>comment()</tt>
         */
        static const SequenceType::Ptr ExactlyOneComment;

        /**
         * <tt>element()*</tt>
         */
        static const SequenceType::Ptr ZeroOrMoreElements;

        /**
         * <tt>document-node()?</tt>
         */
        static const SequenceType::Ptr ZeroOrOneDocumentNode;

        /**
         * <tt>document-node()</tt>
         */
        static const SequenceType::Ptr ExactlyOneDocumentNode;

        /**
         * Identifiers all values which the Effective %Boolean Value
         * can be extracted from.
         */
        static const SequenceType::Ptr EBV;

        /**
         * <tt>xs:anyURI?</tt>
         */
        static const SequenceType::Ptr ZeroOrOneAnyURI;

        /**
         * <tt>xs:hexBinary</tt>
         */
        static const SequenceType::Ptr ExactlyOneHexBinary;

        /**
         * <tt>xs:base64Binary</tt>
         */
        static const SequenceType::Ptr ExactlyOneBase64Binary;

        /**
         * <tt>xs:date</tt>
         */
        static const SequenceType::Ptr ExactlyOneDate;

        /**
         * <tt>xs:dateTime</tt>
         */
        static const SequenceType::Ptr ExactlyOneDateTime;

        /**
         * <tt>xs:dayTimeDuration</tt>
         */
        static const SequenceType::Ptr ExactlyOneDayTimeDuration;

        /**
         * <tt>xs:duration</tt>
         */
        static const SequenceType::Ptr ExactlyOneDuration;

        /**
         * <tt>xs:gDay</tt>
         */
        static const SequenceType::Ptr ExactlyOneGDay;

        /**
         * <tt>xs:gMonth</tt>
         */
        static const SequenceType::Ptr ExactlyOneGMonth;

        /**
         * <tt>xs:gMonthDay</tt>
         */
        static const SequenceType::Ptr ExactlyOneGMonthDay;

        /**
         * <tt>xs:gYear</tt>
         */
        static const SequenceType::Ptr ExactlyOneGYear;

        /**
         * <tt>xs:gYearMonth</tt>
         */
        static const SequenceType::Ptr ExactlyOneGYearMonth;

        /**
         * <tt>xs:yearMonthDuration</tt>
         */
        static const SequenceType::Ptr ExactlyOneYearMonthDuration;

        /**
         * <tt>xs:time</tt>
         */
        static const SequenceType::Ptr ExactlyOneTime;

        /**
         * <tt>xs:time?</tt>
         */
        static const SequenceType::Ptr ZeroOrOneDate;

        /**
         * <tt>xs:dateTime?</tt>
         */
        static const SequenceType::Ptr ZeroOrOneDateTime;

        /**
         * <tt>xs:dayTimeDuration?</tt>
         */
        static const SequenceType::Ptr ZeroOrOneDayTimeDuration;

        /**
         * <tt>xs:duration?</tt>
         */
        static const SequenceType::Ptr ZeroOrOneDuration;

        /**
         * <tt>xs:time?</tt>
         */
        static const SequenceType::Ptr ZeroOrOneTime;

        /**
         * <tt>xs:yearMonthDuration?</tt>
         */
        static const SequenceType::Ptr ZeroOrOneYearMonthDuration;

    private:
        /**
         * The constructor is private and has no implementation,
         * because this class is not meant to be instantiated.
         *
         * It should only be used via its static members.
         */
        inline CommonSequenceTypes();

        Q_DISABLE_COPY(CommonSequenceTypes)
    };
}

QT_END_HEADER 

#endif

// vim: et:ts=4:sw=4:sts=4

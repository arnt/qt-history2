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

#ifndef Patternist_ExternalVariableLoader_H
#define Patternist_ExternalVariableLoader_H

#include "qitem_p.h"
#include "qsequencetype_p.h"
#include "qqname_p.h"

QT_BEGIN_HEADER 

QT_BEGIN_NAMESPACE

namespace Patternist
{
    class DynamicContext;

    /**
     * @short Responsible for loading and declaring available external variables.
     *
     * An external variable in XQuery is a global variable that has been declared to receive
     * its value from the XQuery implementation, as opposed to an initializing expression. Here
     * is an example of a query with an external variable declaration, followed by a ordinary
     * global variable:
     *
     * <tt> declare variable $theName external;
     * declare variable $theName := "the value";
     * "And here's the query body(a string literal)"</tt>
     *
     * An external variable declaration can also specify a sequence type:
     *
     * <tt>declare variable $theName as xs:integer external;</tt>
     *
     * This class allows the user to supply the values to external variables. When
     * an external variable declaration occur in the query,
     * announceExternalVariable() is called.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class Q_AUTOTEST_EXPORT ExternalVariableLoader : public QSharedData
    {
    public:
        typedef PlainSharedPtr<ExternalVariableLoader> Ptr;
        inline ExternalVariableLoader() {}

        virtual ~ExternalVariableLoader();

        /**
         * Called when Patternist encounters an external variable in the query. It is guaranteed
         * to be called once for each external variable appearing in a query module.
         *
         * @param name the name of the variable. Quaranteed to never be @c null.
         * @param declaredType the type that the user declared the variable to be of. Whether
         * this type matches the actual value of the variable or not is irrelevant. Patternist
         * will do the necessary error handling based on the sequence type that is returned from
         * this function. If the user didn't declare a type, the type is <tt>item()*</tt>(zero or
         * more items). Quaranteed to never be @c null.
         * @returns the sequence type of the value this ExternalVariableLoader actually supplies. However,
         * if the ExternalVariableLoader knows it cannot supply a variable by this name, @c null should be
         * returned.
         */
        virtual SequenceType::Ptr announceExternalVariable(const QName name,
                                                           const SequenceType::Ptr &declaredType);

        /**
         * This function is called at runtime when the external variable by name @p name needs
         * to be evaluated. It is not defined how many times this function will be called. It
         * depends on aspects such as how the query was optimized.
         *
         * @param name the name of the variable. Quaranteed to never be @c null.
         * @param context the DynamicContext.
         * @returns the value of the variable. Remember that this value must match the
         * sequence type returned from announceExternalVariable() for the same name.
         */
        virtual Item::Iterator::Ptr evaluateSequence(const QName name,
                                                     const PlainSharedPtr<DynamicContext> &context);

        virtual Item evaluateSingleton(const QName name,
                                            const PlainSharedPtr<DynamicContext> &context);
        virtual bool evaluateEBV(const QName name,
                                 const PlainSharedPtr<DynamicContext> &context);
    };
}

QT_END_NAMESPACE

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

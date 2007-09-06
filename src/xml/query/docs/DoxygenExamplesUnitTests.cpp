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
***************************************************************************
*/

#include <QtTest/QtTest>

#include "AtomicComparator.h"
#include "Boolean.h"
#include "BuiltinTypes.h"
#include "ListIterator.h"
#include "RangeIterator.h"
#include "ReportContext.h"
#include "EmptyContainer.h"
#include "XPathHelper.h"

#include "DoxygenExamplesUnitTests.h"

using namespace Patternist;

QTEST_MAIN(DoxygenExamplesUnitTests)

/**
 * @short This class declaration is needed in Example-Expression-typeCheck.cpp
 * @internal
 * @author Frans Englich <fenglich@trolltech.com>
 */
class MyExpression : public EmptyContainer
{
public:
    virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                      const SequenceType::Ptr &reqType);
};

#include "Example-Expression-typeCheck.cpp"

void DoxygenExamplesUnitTests::runTests()
{
/* Alphabetically. */
#include "Example-AtomicComparator-displayName.cpp"
#include "Example-AtomicValue-Boolean.cpp"
#include "Example-ItemType-operator.cpp"
#include "Example-RangeIterator-RangeIterator.cpp"
#include "Example-ReportContext-codeToString.cpp"
#include "Example-XPathHelper-splitQName.cpp"

    QCOMPARE(gName,         QLatin1String("="));
    QCOMPARE(vName,         QLatin1String("eq"));

    QCOMPARE(errorString,   QLatin1String("XPDY0002"));

    QVERIFY(boolFromLex->evaluateEBV(DynamicContext::Ptr()));
    QVERIFY(!boolFromVal->evaluateEBV(DynamicContext::Ptr()));
    QVERIFY(lexicalError->hasError());
}

// vim: et:ts=4:sw=4:sts=4

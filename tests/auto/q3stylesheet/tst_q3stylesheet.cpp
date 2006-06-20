/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <qapplication.h>
#include <qdebug.h>
#include <q3stylesheet.h>


//TESTED_CLASS=
//TESTED_FILES=q3stylesheet.h

class tst_Q3StyleSheet : public QObject
{
Q_OBJECT

public:
    tst_Q3StyleSheet();
    virtual ~tst_Q3StyleSheet();

private slots:
    void getSetCheck();
};

tst_Q3StyleSheet::tst_Q3StyleSheet()
{
}

tst_Q3StyleSheet::~tst_Q3StyleSheet()
{
}

// Testing get/set functions
void tst_Q3StyleSheet::getSetCheck()
{
    Q3StyleSheet ss;
    Q3StyleSheetItem *obj1 = new Q3StyleSheetItem(&ss, "Stylesheet Item");
    // DisplayMode Q3StyleSheetItem::displayMode()
    // void Q3StyleSheetItem::setDisplayMode(DisplayMode)
    obj1->setDisplayMode(Q3StyleSheetItem::DisplayMode(Q3StyleSheetItem::DisplayInline));
    QCOMPARE(Q3StyleSheetItem::DisplayMode(Q3StyleSheetItem::DisplayInline), obj1->displayMode());
    obj1->setDisplayMode(Q3StyleSheetItem::DisplayMode(Q3StyleSheetItem::DisplayListItem));
    QCOMPARE(Q3StyleSheetItem::DisplayMode(Q3StyleSheetItem::DisplayListItem), obj1->displayMode());
    obj1->setDisplayMode(Q3StyleSheetItem::DisplayMode(Q3StyleSheetItem::DisplayNone));
    QCOMPARE(Q3StyleSheetItem::DisplayMode(Q3StyleSheetItem::DisplayNone), obj1->displayMode());
    obj1->setDisplayMode(Q3StyleSheetItem::DisplayMode(Q3StyleSheetItem::DisplayBlock));
    QCOMPARE(Q3StyleSheetItem::DisplayMode(Q3StyleSheetItem::DisplayBlock), obj1->displayMode());

    // int Q3StyleSheetItem::alignment()
    // void Q3StyleSheetItem::setAlignment(int)
    obj1->setAlignment(0);
    QCOMPARE(0, obj1->alignment());
    obj1->setAlignment(INT_MIN);
    QCOMPARE(INT_MIN, obj1->alignment());
    obj1->setAlignment(INT_MAX);
    QCOMPARE(INT_MAX, obj1->alignment());

    // VerticalAlignment Q3StyleSheetItem::verticalAlignment()
    // void Q3StyleSheetItem::setVerticalAlignment(VerticalAlignment)
    obj1->setVerticalAlignment(Q3StyleSheetItem::VerticalAlignment(Q3StyleSheetItem::VAlignBaseline));
    QCOMPARE(Q3StyleSheetItem::VerticalAlignment(Q3StyleSheetItem::VAlignBaseline), obj1->verticalAlignment());
    obj1->setVerticalAlignment(Q3StyleSheetItem::VerticalAlignment(Q3StyleSheetItem::VAlignSub));
    QCOMPARE(Q3StyleSheetItem::VerticalAlignment(Q3StyleSheetItem::VAlignSub), obj1->verticalAlignment());
    obj1->setVerticalAlignment(Q3StyleSheetItem::VerticalAlignment(Q3StyleSheetItem::VAlignSuper));
    QCOMPARE(Q3StyleSheetItem::VerticalAlignment(Q3StyleSheetItem::VAlignSuper), obj1->verticalAlignment());

    // int Q3StyleSheetItem::fontWeight()
    // void Q3StyleSheetItem::setFontWeight(int)
    obj1->setFontWeight(0);
    QCOMPARE(0, obj1->fontWeight());
    obj1->setFontWeight(INT_MIN);
    QCOMPARE(0, obj1->fontWeight()); // Range<0, 99>
    obj1->setFontWeight(INT_MAX);
    QCOMPARE(99, obj1->fontWeight()); // Range<0, 99>

    // int Q3StyleSheetItem::logicalFontSize()
    // void Q3StyleSheetItem::setLogicalFontSize(int)
    obj1->setLogicalFontSize(0);
    QCOMPARE(1, obj1->logicalFontSize()); // Range<1, 7>
    obj1->setLogicalFontSize(INT_MIN);
    QCOMPARE(1, obj1->logicalFontSize()); // Range<1, 7>
    obj1->setLogicalFontSize(INT_MAX);
    QCOMPARE(7, obj1->logicalFontSize());  // Range<1, 7>

    // int Q3StyleSheetItem::logicalFontSizeStep()
    // void Q3StyleSheetItem::setLogicalFontSizeStep(int)
    obj1->setLogicalFontSizeStep(0);
    QCOMPARE(0, obj1->logicalFontSizeStep());
    obj1->setLogicalFontSizeStep(INT_MIN);
    QCOMPARE(INT_MIN, obj1->logicalFontSizeStep());
    obj1->setLogicalFontSizeStep(INT_MAX);
    QCOMPARE(INT_MAX, obj1->logicalFontSizeStep());

    // int Q3StyleSheetItem::fontSize()
    // void Q3StyleSheetItem::setFontSize(int)
    obj1->setFontSize(0);
    QCOMPARE(0, obj1->fontSize());
    obj1->setFontSize(INT_MIN);
    QCOMPARE(int(Q3StyleSheetItem::Undefined), obj1->fontSize()); // Expect an undefined return value for non-valid point size, as per docs
    obj1->setFontSize(INT_MAX);
    QCOMPARE(INT_MAX, obj1->fontSize());

    // int Q3StyleSheetItem::numberOfColumns()
    // void Q3StyleSheetItem::setNumberOfColumns(int)
    int currentNumCols = obj1->numberOfColumns();
    obj1->setNumberOfColumns(0);
    QCOMPARE(currentNumCols, obj1->numberOfColumns()); // Can't set 0 column count
    obj1->setNumberOfColumns(INT_MIN);
    QCOMPARE(currentNumCols, obj1->numberOfColumns()); // Can't set negative column count
    obj1->setNumberOfColumns(INT_MAX);
    QCOMPARE(INT_MAX, obj1->numberOfColumns());

    // WhiteSpaceMode Q3StyleSheetItem::whiteSpaceMode()
    // void Q3StyleSheetItem::setWhiteSpaceMode(WhiteSpaceMode)
    obj1->setWhiteSpaceMode(Q3StyleSheetItem::WhiteSpaceMode(Q3StyleSheetItem::WhiteSpaceNormal));
    QCOMPARE(Q3StyleSheetItem::WhiteSpaceMode(Q3StyleSheetItem::WhiteSpaceNormal), obj1->whiteSpaceMode());
    obj1->setWhiteSpaceMode(Q3StyleSheetItem::WhiteSpaceMode(Q3StyleSheetItem::WhiteSpacePre));
    QCOMPARE(Q3StyleSheetItem::WhiteSpaceMode(Q3StyleSheetItem::WhiteSpacePre), obj1->whiteSpaceMode());
    obj1->setWhiteSpaceMode(Q3StyleSheetItem::WhiteSpaceMode(Q3StyleSheetItem::WhiteSpaceNoWrap));
    QCOMPARE(Q3StyleSheetItem::WhiteSpaceMode(Q3StyleSheetItem::WhiteSpaceNoWrap), obj1->whiteSpaceMode());

    // int Q3StyleSheetItem::lineSpacing()
    // void Q3StyleSheetItem::setLineSpacing(int)
    obj1->setLineSpacing(0);
    QCOMPARE(0, obj1->lineSpacing());
    obj1->setLineSpacing(INT_MIN);
    QCOMPARE(0, obj1->lineSpacing()); // Should not be able to set negative line spacing(?)
    obj1->setLineSpacing(INT_MAX);
    QCOMPARE(INT_MAX, obj1->lineSpacing());
}

QTEST_MAIN(tst_Q3StyleSheet)
#include "tst_q3stylesheet.moc"

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
    QCOMPARE(obj1->displayMode(), Q3StyleSheetItem::DisplayMode(Q3StyleSheetItem::DisplayInline));
    obj1->setDisplayMode(Q3StyleSheetItem::DisplayMode(Q3StyleSheetItem::DisplayListItem));
    QCOMPARE(obj1->displayMode(), Q3StyleSheetItem::DisplayMode(Q3StyleSheetItem::DisplayListItem));
    obj1->setDisplayMode(Q3StyleSheetItem::DisplayMode(Q3StyleSheetItem::DisplayNone));
    QCOMPARE(obj1->displayMode(), Q3StyleSheetItem::DisplayMode(Q3StyleSheetItem::DisplayNone));
    obj1->setDisplayMode(Q3StyleSheetItem::DisplayMode(Q3StyleSheetItem::DisplayBlock));
    QCOMPARE(obj1->displayMode(), Q3StyleSheetItem::DisplayMode(Q3StyleSheetItem::DisplayBlock));

    // int Q3StyleSheetItem::alignment()
    // void Q3StyleSheetItem::setAlignment(int)
    obj1->setAlignment(0);
    QCOMPARE(obj1->alignment(), 0);
    obj1->setAlignment(INT_MIN);
    QCOMPARE(obj1->alignment(), INT_MIN);
    obj1->setAlignment(INT_MAX);
    QCOMPARE(obj1->alignment(), INT_MAX);

    // VerticalAlignment Q3StyleSheetItem::verticalAlignment()
    // void Q3StyleSheetItem::setVerticalAlignment(VerticalAlignment)
    obj1->setVerticalAlignment(Q3StyleSheetItem::VerticalAlignment(Q3StyleSheetItem::VAlignBaseline));
    QCOMPARE(obj1->verticalAlignment(), Q3StyleSheetItem::VerticalAlignment(Q3StyleSheetItem::VAlignBaseline));
    obj1->setVerticalAlignment(Q3StyleSheetItem::VerticalAlignment(Q3StyleSheetItem::VAlignSub));
    QCOMPARE(obj1->verticalAlignment(), Q3StyleSheetItem::VerticalAlignment(Q3StyleSheetItem::VAlignSub));
    obj1->setVerticalAlignment(Q3StyleSheetItem::VerticalAlignment(Q3StyleSheetItem::VAlignSuper));
    QCOMPARE(obj1->verticalAlignment(), Q3StyleSheetItem::VerticalAlignment(Q3StyleSheetItem::VAlignSuper));

    // int Q3StyleSheetItem::fontWeight()
    // void Q3StyleSheetItem::setFontWeight(int)
    obj1->setFontWeight(0);
    QCOMPARE(obj1->fontWeight(), 0);
    obj1->setFontWeight(INT_MIN);
    // Should return 0, but we cannot change this behavior in a Qt3Support class.
    QCOMPARE(obj1->fontWeight(), INT_MIN); // Range<0, 99>
    obj1->setFontWeight(INT_MAX);
    // Should return 99, but we cannot change this behavior in a Qt3Support class.
    QCOMPARE(obj1->fontWeight(), INT_MAX); // Range<0, 99>

    // int Q3StyleSheetItem::logicalFontSize()
    // void Q3StyleSheetItem::setLogicalFontSize(int)
    obj1->setLogicalFontSize(0);
    // Should return 1, but we cannot change this behavior in a Qt3Support class.
    QCOMPARE(obj1->logicalFontSize(), 0); // Range<1, 7>
    obj1->setLogicalFontSize(INT_MIN);
    // Should return 1, but we cannot change this behavior in a Qt3Support class.
    QCOMPARE(obj1->logicalFontSize(), INT_MIN); // Range<1, 7>
    obj1->setLogicalFontSize(INT_MAX);
    // Should return 7, but we cannot change this behavior in a Qt3Support class.
    QCOMPARE(obj1->logicalFontSize(), INT_MAX);  // Range<1, 7>

    // int Q3StyleSheetItem::logicalFontSizeStep()
    // void Q3StyleSheetItem::setLogicalFontSizeStep(int)
    obj1->setLogicalFontSizeStep(0);
    QCOMPARE(obj1->logicalFontSizeStep(), 0);
    obj1->setLogicalFontSizeStep(INT_MIN);
    QCOMPARE(obj1->logicalFontSizeStep(), INT_MIN);
    obj1->setLogicalFontSizeStep(INT_MAX);
    QCOMPARE(obj1->logicalFontSizeStep(), INT_MAX);

    // int Q3StyleSheetItem::fontSize()
    // void Q3StyleSheetItem::setFontSize(int)
    obj1->setFontSize(0);
    QCOMPARE(obj1->fontSize(), 0);
    obj1->setFontSize(INT_MIN);
    // Should return -1, but we cannot change this behavior in a Qt3Support class.
    QCOMPARE(obj1->fontSize(), INT_MIN); // Expect an undefined return value for non-valid point size, as per docs
    obj1->setFontSize(INT_MAX);
    QCOMPARE(obj1->fontSize(), INT_MAX);

    // int Q3StyleSheetItem::numberOfColumns()
    // void Q3StyleSheetItem::setNumberOfColumns(int)
    int currentNumCols = obj1->numberOfColumns();
    obj1->setNumberOfColumns(0);
    QCOMPARE(obj1->numberOfColumns(), currentNumCols); // Can't set 0 column count
    obj1->setNumberOfColumns(INT_MIN);
    QCOMPARE(obj1->numberOfColumns(), currentNumCols); // Can't set negative column count
    obj1->setNumberOfColumns(INT_MAX);
    QCOMPARE(obj1->numberOfColumns(), INT_MAX);

    // WhiteSpaceMode Q3StyleSheetItem::whiteSpaceMode()
    // void Q3StyleSheetItem::setWhiteSpaceMode(WhiteSpaceMode)
    obj1->setWhiteSpaceMode(Q3StyleSheetItem::WhiteSpaceMode(Q3StyleSheetItem::WhiteSpaceNormal));
    QCOMPARE(obj1->whiteSpaceMode(), Q3StyleSheetItem::WhiteSpaceMode(Q3StyleSheetItem::WhiteSpaceNormal));
    obj1->setWhiteSpaceMode(Q3StyleSheetItem::WhiteSpaceMode(Q3StyleSheetItem::WhiteSpacePre));
    QCOMPARE(obj1->whiteSpaceMode(), Q3StyleSheetItem::WhiteSpaceMode(Q3StyleSheetItem::WhiteSpacePre));
    obj1->setWhiteSpaceMode(Q3StyleSheetItem::WhiteSpaceMode(Q3StyleSheetItem::WhiteSpaceNoWrap));
    QCOMPARE(obj1->whiteSpaceMode(), Q3StyleSheetItem::WhiteSpaceMode(Q3StyleSheetItem::WhiteSpaceNoWrap));

    // int Q3StyleSheetItem::lineSpacing()
    // void Q3StyleSheetItem::setLineSpacing(int)
    obj1->setLineSpacing(0);
    QCOMPARE(obj1->lineSpacing(), 0);
    obj1->setLineSpacing(INT_MIN);
    // Should return -1, but we cannot change this behavior in a Qt3Support class.
    QCOMPARE(obj1->lineSpacing(), INT_MIN); // Should not be able to set negative line spacing(?)
    obj1->setLineSpacing(INT_MAX);
    QCOMPARE(obj1->lineSpacing(), INT_MAX);
}

QTEST_MAIN(tst_Q3StyleSheet)
#include "tst_q3stylesheet.moc"

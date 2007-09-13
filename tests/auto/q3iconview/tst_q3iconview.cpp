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
#include <q3iconview.h>

//TESTED_FILES=

QT_DECLARE_CLASS(Q3IconView)

class tst_Q3IconView : public QObject
{
    Q_OBJECT

public:
    tst_Q3IconView();

private slots:
    void isRenaming();

private:
    Q3IconView *testWidget;
};

tst_Q3IconView::tst_Q3IconView()
{
}

void tst_Q3IconView::isRenaming()
{
    Q3IconView view;
    QVERIFY( !view.isRenaming() );
    Q3IconViewItem *item = new Q3IconViewItem( &view, "Test" );
    item->setRenameEnabled( TRUE );
    item->rename();
    QVERIFY( view.isRenaming() );
}


QTEST_MAIN(tst_Q3IconView)
#include "tst_q3iconview.moc"


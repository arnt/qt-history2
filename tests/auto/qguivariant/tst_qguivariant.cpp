/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <qvariant.h>



class tst_QGuiVariant : public QObject
{
    Q_OBJECT

public:
    tst_QGuiVariant();

private slots:
    void variantWithoutApplication();
};

tst_QGuiVariant::tst_QGuiVariant()
{}

void tst_QGuiVariant::variantWithoutApplication()
{
    QVariant v = QString("red");

    QVERIFY(qvariant_cast<QColor>(v) == QColor(Qt::red));
}


QTEST_APPLESS_MAIN(tst_QGuiVariant)
#include "tst_qguivariant.moc"

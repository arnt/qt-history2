/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtCore/QtCore>
#include <QtTest/QtTest>

class MaxWarnings: public QObject
{
    Q_OBJECT
private slots:
    void warn();
};

void MaxWarnings::warn()
{
    for (int i = 0; i < 10000; ++i)
        qWarning("%d", i);
}

QTEST_MAIN(MaxWarnings)
#include "maxwarnings.moc"

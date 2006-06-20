/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QtGui>

class tst_QSound : public QObject
{ 
    Q_OBJECT

public:
    tst_QSound( QObject* parent=0) : QObject(parent) {}

private slots:
        void checkFinished();
};

void tst_QSound::checkFinished()
{
            QSound sound("4.wav");
            sound.setLoops(3);
            sound.play();
            QTest::qWait(5000);
            QVERIFY(sound.isFinished() );
}

QTEST_MAIN(tst_QSound);
#include "tst_qsound.moc"

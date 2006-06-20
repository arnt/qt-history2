/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtGui>
#include <QtNetwork>

#include <QtTest/QtTest>


#include "mainwindow.h"

class tst_QtWidgets: public QObject
{
    Q_OBJECT

private slots:
    void snapshot();
};


void tst_QtWidgets::snapshot()
{
    StyleWidget widget(0, Qt::X11BypassWindowManagerHint);
    widget.show();

    QPixmap pix = QPixmap::grabWidget(&widget);

    QVERIFY(!pix.isNull());

    QBuffer buf;
    pix.save(&buf, "PNG");
    QVERIFY(buf.size() > 0);

    QString filename = "qtwidgets_" + QHostInfo::localHostName() + "_" + QDateTime::currentDateTime().toString("yyyy.MM.dd_hh.mm.ss") + ".png";

    QFtp ftp;
    ftp.connectToHost("kramer.troll.no");
    ftp.login("anonymous");
    ftp.cd("pics");
    ftp.put(buf.data(), filename, QFtp::Binary);
    ftp.close();

    int i = 0;
    while (i < 100 && ftp.hasPendingCommands()) {
        QCoreApplication::instance()->processEvents();
        QTest::qWait(250);
        ++i;
    }
    QVERIFY2(ftp.error() == QFtp::NoError, ftp.errorString().toLocal8Bit().constData());
    QVERIFY(!ftp.hasPendingCommands());
}



QTEST_MAIN(tst_QtWidgets)

#include "tst_qtwidgets.moc"

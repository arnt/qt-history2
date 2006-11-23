/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#ifdef Q_WS_QWS

#include <QWSEmbedWidget>
#include <QLabel>

class tst_QWSEmbedWidget : public QObject
{
    Q_OBJECT

public:
    tst_QWSEmbedWidget() {}
    ~tst_QWSEmbedWidget() {}

private slots:
    void embedWidget();
};

void tst_QWSEmbedWidget::embedWidget()
{
    QLabel embedded("hello");
    embedded.show();
    QApplication::processEvents();
    QVERIFY(embedded.isVisible());

    {
        QWSEmbedWidget embedder(embedded.winId());
        embedder.show();
        QApplication::processEvents();
        QVERIFY(embedded.isVisible());
    }
    QApplication::processEvents();
    QVERIFY(!embedded.isVisible());

    {
        QWidget w;
        embedded.setWindowFlags(Qt::FramelessWindowHint);
        QWSEmbedWidget embedder(embedded.winId(), &w);

        const QRect geometry(100, 100, 100, 100);
        embedder.setGeometry(geometry);
        w.show();

        QApplication::processEvents();

        const QPoint offset = w.mapToGlobal(QPoint(0, 0));
        QCOMPARE(embedded.geometry(), geometry.translated(offset));
        QVERIFY(embedded.isVisible());
    }
    QApplication::processEvents();
    QVERIFY(!embedded.isVisible());
}

QTEST_MAIN(tst_QWSEmbedWidget)

#include "tst_qwsembedwidget.moc"

#else // Q_WS_QWS
QTEST_NOOP_MAIN
#endif

/****************************************************************************
  **
  ** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
  **
  ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
  ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
  **
  ****************************************************************************/

#include <QtTest/QtTest>
#include <QSizeGrip>
#include <QEvent>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QLabel>

#if defined(Q_WS_X11)
extern void qt_x11_wait_for_window_manager(QWidget *w);
#endif

static inline Qt::Corner sizeGripCorner(QWidget *parent, QSizeGrip *sizeGrip)
{
    if (!parent || !sizeGrip)
        return Qt::TopLeftCorner;

    const QPoint sizeGripPos = sizeGrip->mapTo(parent, QPoint(0, 0));
    bool isAtBottom = sizeGripPos.y() >= parent->height() / 2;
    bool isAtLeft = sizeGripPos.x() <= parent->width() / 2;
    if (isAtLeft)
        return isAtBottom ? Qt::BottomLeftCorner : Qt::TopLeftCorner;
    else
        return isAtBottom ? Qt::BottomRightCorner : Qt::TopRightCorner;

}

//TESTED_CLASS=QSizeGrip
//TESTED_FILES=gui/widgets/qsizegrip.cpp

Q_DECLARE_METATYPE(Qt::WindowType);

class tst_QSizeGrip : public QObject
{
    Q_OBJECT
public slots:
    void initTestCase();
    void cleanupTestCase();

private slots:
    void hideAndShowOnWindowStateChange_data();
    void hideAndShowOnWindowStateChange();
    void orientation();

private:
    QLineEdit *dummyWidget;
};

class TestWidget : public QWidget
{
public:
    TestWidget(QWidget *parent = 0, Qt::WindowFlags flags = 0) : QWidget(parent, flags) {}
    QSize sizeHint() const { return QSize(300, 200); }
    void changeEvent(QEvent *event)
    {
        QWidget::changeEvent(event);
        if (isWindow() && event->type() == QEvent::WindowStateChange) {
#ifdef Q_WS_X11
            qt_x11_wait_for_window_manager(this);
#endif
        }
    }
};

void tst_QSizeGrip::initTestCase()
{
    dummyWidget = new QLineEdit;
    dummyWidget->show();
}

void tst_QSizeGrip::cleanupTestCase()
{
    delete dummyWidget;
    dummyWidget = 0;
}

void tst_QSizeGrip::hideAndShowOnWindowStateChange_data()
{
    QTest::addColumn<Qt::WindowType>("windowType");
    QTest::newRow("Qt::Window") << Qt::Window;
    QTest::newRow("Qt::SubWindow") << Qt::SubWindow;
}

void tst_QSizeGrip::hideAndShowOnWindowStateChange()
{
    QFETCH(Qt::WindowType, windowType);

    QWidget *parentWidget = windowType == Qt::Window ?  0 : new QWidget;
    TestWidget *widget = new TestWidget(parentWidget, Qt::WindowFlags(windowType));
    QSizeGrip *sizeGrip = new QSizeGrip(widget);

    // Normal.
    if (parentWidget)
        parentWidget->show();
    else
        widget->show();
    QVERIFY(sizeGrip->isVisible());

    widget->showFullScreen();
    QVERIFY(!sizeGrip->isVisible());

    widget->showNormal();
    QVERIFY(sizeGrip->isVisible());

    widget->showMaximized();
#ifndef Q_WS_MAC
    QVERIFY(!sizeGrip->isVisible());
#else
    QVERIFY(sizeGrip->isVisible());
#endif

    widget->showNormal();
    QVERIFY(sizeGrip->isVisible());

    sizeGrip->hide();
    QVERIFY(!sizeGrip->isVisible());

    widget->showFullScreen();
    widget->showNormal();
    QVERIFY(!sizeGrip->isVisible());
    widget->showMaximized();
    widget->showNormal();
    QVERIFY(!sizeGrip->isVisible());

    delete widget;
    delete parentWidget;
}

void tst_QSizeGrip::orientation()
{
    TestWidget widget;
    widget.setLayout(new QVBoxLayout);
    QSizeGrip *sizeGrip = new QSizeGrip(&widget);
    sizeGrip->setFixedSize(sizeGrip->sizeHint());
    widget.layout()->addWidget(sizeGrip);
    widget.layout()->setAlignment(sizeGrip, Qt::AlignBottom | Qt::AlignRight);

    widget.show();
    QCOMPARE(sizeGripCorner(&widget, sizeGrip), Qt::BottomRightCorner);

    widget.setLayoutDirection(Qt::RightToLeft);
    qApp->processEvents();
    QCOMPARE(sizeGripCorner(&widget, sizeGrip), Qt::BottomLeftCorner);

    widget.unsetLayoutDirection();
    qApp->processEvents();
    QCOMPARE(sizeGripCorner(&widget, sizeGrip), Qt::BottomRightCorner);

    widget.layout()->setAlignment(sizeGrip, Qt::AlignTop | Qt::AlignRight);
    qApp->processEvents();
    QCOMPARE(sizeGripCorner(&widget, sizeGrip), Qt::TopRightCorner);

    widget.setLayoutDirection(Qt::RightToLeft);
    qApp->processEvents();
    QCOMPARE(sizeGripCorner(&widget, sizeGrip), Qt::TopLeftCorner);

    widget.unsetLayoutDirection();
    qApp->processEvents();
    QCOMPARE(sizeGripCorner(&widget, sizeGrip), Qt::TopRightCorner);
}

QTEST_MAIN(tst_QSizeGrip)
#include "tst_qsizegrip.moc"


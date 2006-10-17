/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QtGui/QDesktopWidget>
#include <QDebug>

//TESTED_CLASS=qdesktopwidget

class tst_QDesktopWidget : public QObject
{
    Q_OBJECT

public:
    tst_QDesktopWidget();
    virtual ~tst_QDesktopWidget();

public slots:
    void init();
    void cleanup();

private slots:
    void numScreens();
    void primaryScreen();
    void screenNumberForQWidget();
    void screenNumberForQPoint();
    void availableGeometry();
};

tst_QDesktopWidget::tst_QDesktopWidget()
{
}

tst_QDesktopWidget::~tst_QDesktopWidget()
{
}

void tst_QDesktopWidget::init()
{
}

void tst_QDesktopWidget::cleanup()
{
}

void tst_QDesktopWidget::numScreens()
{
    QDesktopWidget desktop;
    QVERIFY(desktop.numScreens() > 0);
}

void tst_QDesktopWidget::primaryScreen()
{
    QDesktopWidget desktop;
    QVERIFY(desktop.primaryScreen() >= 0);
    QVERIFY(desktop.primaryScreen() < desktop.numScreens());
}

void tst_QDesktopWidget::availableGeometry()
{
    QDesktopWidget desktop;

    QRect total = desktop.screenGeometry();
    QRect available = desktop.availableGeometry();

    QVERIFY(total.contains(available));
    QCOMPARE(desktop.availableGeometry(desktop.primaryScreen()), available);
    QCOMPARE(desktop.screenGeometry(desktop.primaryScreen()), total);
}

void tst_QDesktopWidget::screenNumberForQWidget()
{
    QDesktopWidget desktop;

    QWidget widget;
    widget.show();
    QApplication::processEvents();
    QVERIFY(widget.isVisible());

    int widgetScreen = desktop.screenNumber(&widget);
    QVERIFY(widgetScreen > -1);
    QVERIFY(widgetScreen < desktop.numScreens());
}

void tst_QDesktopWidget::screenNumberForQPoint()
{
    // make sure QDesktopWidget::screenNumber(QPoint) returns the correct screen
    QDesktopWidget *desktopWidget = QApplication::desktop();
    QRect allScreens;
    for (int i = 0; i < desktopWidget->numScreens(); ++i) {
        QRect screenGeometry = desktopWidget->screenGeometry(i);
        QCOMPARE(desktopWidget->screenNumber(screenGeometry.center()), i);
        allScreens |= screenGeometry;
    }

    // make sure QDesktopWidget::screenNumber(QPoint) returns a valid screen for points that aren't on any screen
    int screen;
    screen = desktopWidget->screenNumber(allScreens.topLeft() - QPoint(1, 1));
    QVERIFY(screen >= 0 && screen < desktopWidget->numScreens());
    screen = desktopWidget->screenNumber(allScreens.topRight() + QPoint(1, 1));
    QVERIFY(screen >= 0 && screen < desktopWidget->numScreens());
    screen = desktopWidget->screenNumber(allScreens.bottomLeft() - QPoint(1, 1));
    QVERIFY(screen >= 0 && screen < desktopWidget->numScreens());
    screen = desktopWidget->screenNumber(allScreens.bottomRight() + QPoint(1, 1));
    QVERIFY(screen >= 0 && screen < desktopWidget->numScreens());
}

QTEST_MAIN(tst_QDesktopWidget)
#include "tst_qdesktopwidget.moc"


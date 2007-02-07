/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/


#include <QtTest/QtTest>


#include <qaction.h>
#include <qdockwidget.h>
#include <qmainwindow.h>

bool hasFeature(QDockWidget *dockwidget, QDockWidget::DockWidgetFeature feature)
{ return (dockwidget->features() & feature) == feature; }

void setFeature(QDockWidget *dockwidget, QDockWidget::DockWidgetFeature feature, bool on = true)
{
    const QDockWidget::DockWidgetFeatures features = dockwidget->features();
    dockwidget->setFeatures(on ? features | feature : features & ~feature);
}

//TESTED_FILES=

class tst_QDockWidget : public QObject
{
     Q_OBJECT

public:
    tst_QDockWidget();

private slots:
    void getSetCheck();
    void widget();
    void setWidget();
    void setFeatures();
    void features();
    void setFloating();
    void setAllowedAreas();
    void allowedAreas();
    void isAreaAllowed();
    void toggleViewAction();
    void featuresChanged();
    void topLevelChanged();
    void allowedAreasChanged();
    void visibilityChanged();
    void dockLocationChanged();
};

// Testing get/set functions
void tst_QDockWidget::getSetCheck()
{
    QDockWidget obj1;
    // QWidget * QDockWidget::widget()
    // void QDockWidget::setWidget(QWidget *)
    QWidget *var1 = new QWidget();
    obj1.setWidget(var1);
    QCOMPARE(var1, obj1.widget());
    obj1.setWidget((QWidget *)0);
    QCOMPARE((QWidget *)0, obj1.widget());
    delete var1;

    // DockWidgetFeatures QDockWidget::features()
    // void QDockWidget::setFeatures(DockWidgetFeatures)
    obj1.setFeatures(QDockWidget::DockWidgetFeatures(QDockWidget::DockWidgetClosable));
    QCOMPARE(QDockWidget::DockWidgetFeatures(QDockWidget::DockWidgetClosable), obj1.features());
    obj1.setFeatures(QDockWidget::DockWidgetFeatures(QDockWidget::DockWidgetMovable));
    QCOMPARE(QDockWidget::DockWidgetFeatures(QDockWidget::DockWidgetMovable), obj1.features());
    obj1.setFeatures(QDockWidget::DockWidgetFeatures(QDockWidget::DockWidgetFloatable));
    QCOMPARE(QDockWidget::DockWidgetFeatures(QDockWidget::DockWidgetFloatable), obj1.features());
    obj1.setFeatures(QDockWidget::DockWidgetFeatures(QDockWidget::AllDockWidgetFeatures));
    QCOMPARE(QDockWidget::DockWidgetFeatures(QDockWidget::AllDockWidgetFeatures), obj1.features());
    obj1.setFeatures(QDockWidget::DockWidgetFeatures(QDockWidget::NoDockWidgetFeatures));
    QCOMPARE(QDockWidget::DockWidgetFeatures(QDockWidget::NoDockWidgetFeatures), obj1.features());
}

tst_QDockWidget::tst_QDockWidget()
{
    qRegisterMetaType<QDockWidget::DockWidgetFeatures>("QDockWidget::DockWidgetFeatures");
    qRegisterMetaType<Qt::DockWidgetAreas>("Qt::DockWidgetAreas");
}

void tst_QDockWidget::widget()
{
    {
        QDockWidget dw;
        QVERIFY(dw.widget() == 0);
    }

    {
        QDockWidget dw;
        QWidget *w1 = new QWidget;
        QWidget *w2 = new QWidget;

        dw.setWidget(w1);
        QVERIFY(dw.widget() != 0);
        QVERIFY(dw.widget() == w1);
        QCOMPARE(w1->parentWidget(), (QWidget*)&dw);

        dw.setWidget(0);
        QVERIFY(dw.widget() == 0);

        dw.setWidget(w2);
        QVERIFY(dw.widget() != 0);
        QVERIFY(dw.widget() == w2);
        QCOMPARE(w2->parentWidget(), (QWidget*)&dw);

        dw.setWidget(0);
        QVERIFY(dw.widget() == 0);

        dw.setWidget(w1);
        QVERIFY(dw.widget() != 0);
        QVERIFY(dw.widget() == w1);
        QCOMPARE(w1->parentWidget(), (QWidget*)&dw);

        dw.setWidget(w2);
        QVERIFY(dw.widget() != 0);
        QVERIFY(dw.widget() == w2);
        QCOMPARE(w2->parentWidget(), (QWidget*)&dw);

        dw.setWidget(0);
        QVERIFY(dw.widget() == 0);
    }

    {
        QDockWidget dw;
        QWidget *w1 = new QWidget;
        QWidget *w2 = new QWidget;

        dw.setWidget(w1);
        QVERIFY(dw.widget() != 0);
        QVERIFY(dw.widget() == w1);
        QCOMPARE(w1->parentWidget(), (QWidget*)&dw);

        w1->setParent(0);
        QVERIFY(dw.widget() == 0);

        dw.setWidget(w2);
        QVERIFY(dw.widget() != 0);
        QVERIFY(dw.widget() == w2);
        QCOMPARE(w2->parentWidget(), (QWidget*)&dw);

        w2->setParent(0);
        QVERIFY(dw.widget() == 0);

        dw.setWidget(w1);
        QVERIFY(dw.widget() != 0);
        QVERIFY(dw.widget() == w1);
        QCOMPARE(w1->parentWidget(), (QWidget*)&dw);

        dw.setWidget(w2);
        QVERIFY(dw.widget() != 0);
        QVERIFY(dw.widget() == w2);
        QCOMPARE(w2->parentWidget(), (QWidget*)&dw);

        w1->setParent(0);
        QVERIFY(dw.widget() != 0);
        QVERIFY(dw.widget() == w2);
        QCOMPARE(w2->parentWidget(), (QWidget*)&dw);

        w2->setParent(0);
        QVERIFY(dw.widget() == 0);
        delete w1;
        delete w2;
    }
}

void tst_QDockWidget::setWidget()
{ DEPENDS_ON("setWidget()"); }

void tst_QDockWidget::setFeatures()
{ DEPENDS_ON("features()"); }

void tst_QDockWidget::features()
{
    QDockWidget dw;

    QSignalSpy spy(&dw, SIGNAL(featuresChanged(QDockWidget::DockWidgetFeatures)));

    // defaults
    QCOMPARE(dw.features(), QDockWidget::AllDockWidgetFeatures);
    QVERIFY(hasFeature(&dw, QDockWidget::DockWidgetClosable));
    QVERIFY(hasFeature(&dw, QDockWidget::DockWidgetMovable));
    QVERIFY(hasFeature(&dw, QDockWidget::DockWidgetFloatable));

    // test individual setting
    setFeature(&dw, QDockWidget::DockWidgetClosable, false);
    QCOMPARE(dw.features(), QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    QVERIFY(!hasFeature(&dw, QDockWidget::DockWidgetClosable));
    QVERIFY(hasFeature(&dw, QDockWidget::DockWidgetMovable));
    QVERIFY(hasFeature(&dw, QDockWidget::DockWidgetFloatable));
    QCOMPARE(spy.count(), 1);
    QCOMPARE((int)*(static_cast<const QDockWidget::DockWidgetFeature *>(spy.at(0).value(0).constData())),
            (int)dw.features());
    spy.clear();
    dw.setFeatures(dw.features());
    QCOMPARE(spy.count(), 0);
    spy.clear();

    setFeature(&dw, QDockWidget::DockWidgetClosable);
    QCOMPARE(dw.features(), QDockWidget::AllDockWidgetFeatures);
    QVERIFY(hasFeature(&dw, QDockWidget::DockWidgetClosable));
    QVERIFY(hasFeature(&dw, QDockWidget::DockWidgetMovable));
    QVERIFY(hasFeature(&dw, QDockWidget::DockWidgetFloatable));
    QCOMPARE(spy.count(), 1);
    QCOMPARE((int)*static_cast<const QDockWidget::DockWidgetFeature *>(spy.at(0).value(0).constData()),
            (int)dw.features());
    spy.clear();
    dw.setFeatures(dw.features());
    QCOMPARE(spy.count(), 0);
    spy.clear();

    setFeature(&dw, QDockWidget::DockWidgetMovable, false);
    QCOMPARE(dw.features(), QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable);
    QVERIFY(hasFeature(&dw, QDockWidget::DockWidgetClosable));
    QVERIFY(!hasFeature(&dw, QDockWidget::DockWidgetMovable));
    QVERIFY(hasFeature(&dw, QDockWidget::DockWidgetFloatable));
    QCOMPARE(spy.count(), 1);
    QCOMPARE((int)*static_cast<const QDockWidget::DockWidgetFeature *>(spy.at(0).value(0).constData()),
            (int)dw.features());
    spy.clear();
    dw.setFeatures(dw.features());
    QCOMPARE(spy.count(), 0);
    spy.clear();

    setFeature(&dw, QDockWidget::DockWidgetMovable);
    QCOMPARE(dw.features(), QDockWidget::AllDockWidgetFeatures);
    QVERIFY(hasFeature(&dw, QDockWidget::DockWidgetClosable));
    QVERIFY(hasFeature(&dw, QDockWidget::DockWidgetMovable));
    QVERIFY(hasFeature(&dw, QDockWidget::DockWidgetFloatable));
    QCOMPARE(spy.count(), 1);
    QCOMPARE((int)*static_cast<const QDockWidget::DockWidgetFeature *>(spy.at(0).value(0).constData()),
            (int)dw.features());
    spy.clear();
    dw.setFeatures(dw.features());
    QCOMPARE(spy.count(), 0);
    spy.clear();

    setFeature(&dw, QDockWidget::DockWidgetFloatable, false);
    QCOMPARE(dw.features(), QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
    QVERIFY(hasFeature(&dw, QDockWidget::DockWidgetClosable));
    QVERIFY(hasFeature(&dw, QDockWidget::DockWidgetMovable));
    QVERIFY(!hasFeature(&dw, QDockWidget::DockWidgetFloatable));
    QCOMPARE(spy.count(), 1);
    QCOMPARE((int)*static_cast<const QDockWidget::DockWidgetFeature *>(spy.at(0).value(0).constData()),
            (int)dw.features());
    spy.clear();
    dw.setFeatures(dw.features());
    QCOMPARE(spy.count(), 0);
    spy.clear();

    setFeature(&dw, QDockWidget::DockWidgetFloatable);
    QCOMPARE(dw.features(), QDockWidget::AllDockWidgetFeatures);
    QVERIFY(hasFeature(&dw, QDockWidget::DockWidgetClosable));
    QVERIFY(hasFeature(&dw, QDockWidget::DockWidgetMovable));
    QVERIFY(hasFeature(&dw, QDockWidget::DockWidgetFloatable));
    QCOMPARE(spy.count(), 1);
    QCOMPARE((int)*static_cast<const QDockWidget::DockWidgetFeature *>(spy.at(0).value(0).constData()),
            (int)dw.features());
    spy.clear();
    dw.setFeatures(dw.features());
    QCOMPARE(spy.count(), 0);
    spy.clear();

    // set all at once
    dw.setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
    QCOMPARE(dw.features(), QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
    QVERIFY(hasFeature(&dw, QDockWidget::DockWidgetClosable));
    QVERIFY(hasFeature(&dw, QDockWidget::DockWidgetMovable));
    QVERIFY(!hasFeature(&dw, QDockWidget::DockWidgetFloatable));
    QCOMPARE(spy.count(), 1);
    QCOMPARE((int)*static_cast<const QDockWidget::DockWidgetFeature *>(spy.at(0).value(0).constData()),
            (int)dw.features());
    spy.clear();
    dw.setFeatures(dw.features());
    QCOMPARE(spy.count(), 0);
    spy.clear();

    dw.setFeatures(QDockWidget::DockWidgetClosable);
    QCOMPARE(dw.features(), QDockWidget::DockWidgetClosable);
    QVERIFY(hasFeature(&dw, QDockWidget::DockWidgetClosable));
    QVERIFY(!hasFeature(&dw, QDockWidget::DockWidgetMovable));
    QVERIFY(!hasFeature(&dw, QDockWidget::DockWidgetFloatable));
    QCOMPARE(spy.count(), 1);
    QCOMPARE((int)*static_cast<const QDockWidget::DockWidgetFeature *>(spy.at(0).value(0).constData()),
            (int)dw.features());
    spy.clear();
    dw.setFeatures(dw.features());
    QCOMPARE(spy.count(), 0);
    spy.clear();

    dw.setFeatures(QDockWidget::AllDockWidgetFeatures);
    QCOMPARE(dw.features(), QDockWidget::AllDockWidgetFeatures);
    QVERIFY(hasFeature(&dw, QDockWidget::DockWidgetClosable));
    QVERIFY(hasFeature(&dw, QDockWidget::DockWidgetMovable));
    QVERIFY(hasFeature(&dw, QDockWidget::DockWidgetFloatable));
    QCOMPARE(spy.count(), 1);
    QCOMPARE((int)*static_cast<const QDockWidget::DockWidgetFeature *>(spy.at(0).value(0).constData()),
            (int)dw.features());
    spy.clear();
    dw.setFeatures(dw.features());
    QCOMPARE(spy.count(), 0);
    spy.clear();
}

void tst_QDockWidget::setFloating()
{
    QMainWindow mw;
    QDockWidget dw;
    mw.addDockWidget(Qt::LeftDockWidgetArea, &dw);

    mw.show();
#ifdef Q_WS_X11
    extern void qt_x11_wait_for_window_manager(QWidget *);
    qt_x11_wait_for_window_manager(&mw);
#endif

    QVERIFY(!dw.isFloating());

    QSignalSpy spy(&dw, SIGNAL(topLevelChanged(bool)));

    dw.setFloating(true);
    QVERIFY(dw.isFloating());
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).value(0).toBool(), dw.isFloating());
    spy.clear();
    dw.setFloating(dw.isFloating());
    QCOMPARE(spy.count(), 0);
    spy.clear();

    dw.setFloating(false);
    QVERIFY(!dw.isFloating());
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).value(0).toBool(), dw.isFloating());
    spy.clear();
    dw.setFloating(dw.isFloating());
    QCOMPARE(spy.count(), 0);
    spy.clear();
}

void tst_QDockWidget::setAllowedAreas()
{ DEPENDS_ON("allowedAreas()"); }

void tst_QDockWidget::allowedAreas()
{
    QDockWidget dw;

    QSignalSpy spy(&dw, SIGNAL(allowedAreasChanged(Qt::DockWidgetAreas)));

    // default
    QCOMPARE(dw.allowedAreas(), Qt::AllDockWidgetAreas);
    QVERIFY(dw.isAreaAllowed(Qt::LeftDockWidgetArea));
    QVERIFY(dw.isAreaAllowed(Qt::RightDockWidgetArea));
    QVERIFY(dw.isAreaAllowed(Qt::TopDockWidgetArea));
    QVERIFY(dw.isAreaAllowed(Qt::BottomDockWidgetArea));

    // a single dock window area
    dw.setAllowedAreas(Qt::LeftDockWidgetArea);
    QCOMPARE(dw.allowedAreas(), Qt::LeftDockWidgetArea);
    QVERIFY(dw.isAreaAllowed(Qt::LeftDockWidgetArea));
    QVERIFY(!dw.isAreaAllowed(Qt::RightDockWidgetArea));
    QVERIFY(!dw.isAreaAllowed(Qt::TopDockWidgetArea));
    QVERIFY(!dw.isAreaAllowed(Qt::BottomDockWidgetArea));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(*static_cast<const Qt::DockWidgetAreas *>(spy.at(0).value(0).constData()),
            dw.allowedAreas());
    spy.clear();
    dw.setAllowedAreas(dw.allowedAreas());
    QCOMPARE(spy.count(), 0);

    dw.setAllowedAreas(Qt::RightDockWidgetArea);
    QCOMPARE(dw.allowedAreas(), Qt::RightDockWidgetArea);
    QVERIFY(!dw.isAreaAllowed(Qt::LeftDockWidgetArea));
    QVERIFY(dw.isAreaAllowed(Qt::RightDockWidgetArea));
    QVERIFY(!dw.isAreaAllowed(Qt::TopDockWidgetArea));
    QVERIFY(!dw.isAreaAllowed(Qt::BottomDockWidgetArea));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(*static_cast<const Qt::DockWidgetAreas *>(spy.at(0).value(0).constData()),
            dw.allowedAreas());
    spy.clear();
    dw.setAllowedAreas(dw.allowedAreas());
    QCOMPARE(spy.count(), 0);

    dw.setAllowedAreas(Qt::TopDockWidgetArea);
    QCOMPARE(dw.allowedAreas(), Qt::TopDockWidgetArea);
    QVERIFY(!dw.isAreaAllowed(Qt::LeftDockWidgetArea));
    QVERIFY(!dw.isAreaAllowed(Qt::RightDockWidgetArea));
    QVERIFY(dw.isAreaAllowed(Qt::TopDockWidgetArea));
    QVERIFY(!dw.isAreaAllowed(Qt::BottomDockWidgetArea));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(*static_cast<const Qt::DockWidgetAreas *>(spy.at(0).value(0).constData()),
            dw.allowedAreas());
    spy.clear();
    dw.setAllowedAreas(dw.allowedAreas());
    QCOMPARE(spy.count(), 0);

    dw.setAllowedAreas(Qt::BottomDockWidgetArea);
    QCOMPARE(dw.allowedAreas(), Qt::BottomDockWidgetArea);
    QVERIFY(!dw.isAreaAllowed(Qt::LeftDockWidgetArea));
    QVERIFY(!dw.isAreaAllowed(Qt::RightDockWidgetArea));
    QVERIFY(!dw.isAreaAllowed(Qt::TopDockWidgetArea));
    QVERIFY(dw.isAreaAllowed(Qt::BottomDockWidgetArea));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(*static_cast<const Qt::DockWidgetAreas *>(spy.at(0).value(0).constData()),
            dw.allowedAreas());
    spy.clear();
    dw.setAllowedAreas(dw.allowedAreas());
    QCOMPARE(spy.count(), 0);

    // multiple dock window areas
    dw.setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
    QCOMPARE(dw.allowedAreas(), Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
    QVERIFY(!dw.isAreaAllowed(Qt::LeftDockWidgetArea));
    QVERIFY(!dw.isAreaAllowed(Qt::RightDockWidgetArea));
    QVERIFY(dw.isAreaAllowed(Qt::TopDockWidgetArea));
    QVERIFY(dw.isAreaAllowed(Qt::BottomDockWidgetArea));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(*static_cast<const Qt::DockWidgetAreas *>(spy.at(0).value(0).constData()),
            dw.allowedAreas());
    spy.clear();
    dw.setAllowedAreas(dw.allowedAreas());
    QCOMPARE(spy.count(), 0);

    dw.setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    QCOMPARE(dw.allowedAreas(), Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    QVERIFY(dw.isAreaAllowed(Qt::LeftDockWidgetArea));
    QVERIFY(dw.isAreaAllowed(Qt::RightDockWidgetArea));
    QVERIFY(!dw.isAreaAllowed(Qt::TopDockWidgetArea));
    QVERIFY(!dw.isAreaAllowed(Qt::BottomDockWidgetArea));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(*static_cast<const Qt::DockWidgetAreas *>(spy.at(0).value(0).constData()),
            dw.allowedAreas());
    spy.clear();
    dw.setAllowedAreas(dw.allowedAreas());
    QCOMPARE(spy.count(), 0);

    dw.setAllowedAreas(Qt::TopDockWidgetArea | Qt::LeftDockWidgetArea);
    QCOMPARE(dw.allowedAreas(), Qt::TopDockWidgetArea | Qt::LeftDockWidgetArea);
    QVERIFY(dw.isAreaAllowed(Qt::LeftDockWidgetArea));
    QVERIFY(!dw.isAreaAllowed(Qt::RightDockWidgetArea));
    QVERIFY(dw.isAreaAllowed(Qt::TopDockWidgetArea));
    QVERIFY(!dw.isAreaAllowed(Qt::BottomDockWidgetArea));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(*static_cast<const Qt::DockWidgetAreas *>(spy.at(0).value(0).constData()),
            dw.allowedAreas());
    spy.clear();
    dw.setAllowedAreas(dw.allowedAreas());
    QCOMPARE(spy.count(), 0);

    dw.setAllowedAreas(Qt::BottomDockWidgetArea | Qt::RightDockWidgetArea);
    QCOMPARE(dw.allowedAreas(), Qt::BottomDockWidgetArea | Qt::RightDockWidgetArea);
    QVERIFY(!dw.isAreaAllowed(Qt::LeftDockWidgetArea));
    QVERIFY(dw.isAreaAllowed(Qt::RightDockWidgetArea));
    QVERIFY(!dw.isAreaAllowed(Qt::TopDockWidgetArea));
    QVERIFY(dw.isAreaAllowed(Qt::BottomDockWidgetArea));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(*static_cast<const Qt::DockWidgetAreas *>(spy.at(0).value(0).constData()),
            dw.allowedAreas());
    spy.clear();
    dw.setAllowedAreas(dw.allowedAreas());
    QCOMPARE(spy.count(), 0);
}

void tst_QDockWidget::isAreaAllowed()
{ DEPENDS_ON("allowedAreas()"); }

void tst_QDockWidget::toggleViewAction()
{
    QMainWindow mw;
    QDockWidget dw(&mw);
    mw.addDockWidget(Qt::LeftDockWidgetArea, &dw);
    mw.show();
    QAction *toggleViewAction = dw.toggleViewAction();
    QVERIFY(!dw.isHidden());
    toggleViewAction->trigger();
    QVERIFY(dw.isHidden());
    toggleViewAction->trigger();
    QVERIFY(!dw.isHidden());
    toggleViewAction->trigger();
    QVERIFY(dw.isHidden());
}

void tst_QDockWidget::visibilityChanged()
{
    QMainWindow mw;
    QDockWidget dw;
    QSignalSpy spy(&dw, SIGNAL(visibilityChanged(bool)));

    mw.addDockWidget(Qt::LeftDockWidgetArea, &dw);
    mw.show();

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toBool(), true);
    spy.clear();

    dw.hide();
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toBool(), false);
    spy.clear();

    dw.hide();
    QCOMPARE(spy.count(), 0);

    dw.show();
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toBool(), true);
    spy.clear();

    dw.show();
    QCOMPARE(spy.count(), 0);

    QDockWidget dw2;
    mw.tabifyDockWidget(&dw, &dw2);
    dw2.show();
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toBool(), false);
    spy.clear();

    dw2.hide();
    qApp->processEvents();
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toBool(), true);
    spy.clear();

    dw2.show();
    qApp->processEvents();
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toBool(), false);
    spy.clear();

    dw.raise();
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toBool(), true);
    spy.clear();

    dw.raise();
    QCOMPARE(spy.count(), 0);

    dw2.raise();
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toBool(), false);
    spy.clear();

    dw2.raise();
    QCOMPARE(spy.count(), 0);

    mw.addDockWidget(Qt::RightDockWidgetArea, &dw2);
    qApp->processEvents();
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toBool(), true);
}

Q_DECLARE_METATYPE(Qt::DockWidgetArea)

void tst_QDockWidget::dockLocationChanged()
{
    qRegisterMetaType<Qt::DockWidgetArea>("Qt::DockWidgetArea");

    QMainWindow mw;
    QDockWidget dw;
    QSignalSpy spy(&dw, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)));

    mw.addDockWidget(Qt::LeftDockWidgetArea, &dw);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(qvariant_cast<Qt::DockWidgetArea>(spy.at(0).at(0)),
                Qt::LeftDockWidgetArea);
    spy.clear();

    mw.addDockWidget(Qt::LeftDockWidgetArea, &dw);
    QCOMPARE(spy.count(), 2);
    QCOMPARE(qvariant_cast<Qt::DockWidgetArea>(spy.at(0).at(0)),
                Qt::NoDockWidgetArea);
    QCOMPARE(qvariant_cast<Qt::DockWidgetArea>(spy.at(1).at(0)),
                Qt::LeftDockWidgetArea);
    spy.clear();

    mw.addDockWidget(Qt::RightDockWidgetArea, &dw);
    QCOMPARE(spy.count(), 2);
    QCOMPARE(qvariant_cast<Qt::DockWidgetArea>(spy.at(0).at(0)),
                Qt::NoDockWidgetArea);
    QCOMPARE(qvariant_cast<Qt::DockWidgetArea>(spy.at(1).at(0)),
                Qt::RightDockWidgetArea);
    spy.clear();

    mw.removeDockWidget(&dw);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(qvariant_cast<Qt::DockWidgetArea>(spy.at(0).at(0)),
                Qt::NoDockWidgetArea);
    spy.clear();

    QDockWidget dw2;
    mw.addDockWidget(Qt::TopDockWidgetArea, &dw2);
    mw.tabifyDockWidget(&dw2, &dw);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(qvariant_cast<Qt::DockWidgetArea>(spy.at(0).at(0)),
                Qt::TopDockWidgetArea);
    spy.clear();

    mw.splitDockWidget(&dw2, &dw, Qt::Horizontal);
    QCOMPARE(spy.count(), 2);
    QCOMPARE(qvariant_cast<Qt::DockWidgetArea>(spy.at(0).at(0)),
                Qt::NoDockWidgetArea);
    QCOMPARE(qvariant_cast<Qt::DockWidgetArea>(spy.at(1).at(0)),
                Qt::TopDockWidgetArea);
    spy.clear();
}

void tst_QDockWidget::featuresChanged()
{ DEPENDS_ON("features()"); }

void tst_QDockWidget::topLevelChanged()
{ DEPENDS_ON("setFloating()"); }

void tst_QDockWidget::allowedAreasChanged()
{ DEPENDS_ON("allowedAreas()"); }

QTEST_MAIN(tst_QDockWidget)
#include "tst_qdockwidget.moc"

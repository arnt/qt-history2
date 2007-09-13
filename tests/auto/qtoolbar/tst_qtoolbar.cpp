/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>


#include <qabstractbutton.h>
#include <qaction.h>
#include <qlayout.h>
#include <qmainwindow.h>
#include <qpixmap.h>
#include <qstyle.h>
#include <qtoolbar.h>
#include <qwidgetaction.h>
#include <qtoolbutton.h>



//TESTED_FILES=

QT_DECLARE_CLASS(QAction)

class tst_QToolBar : public QObject
{
    Q_OBJECT

public:
    tst_QToolBar();

public slots:
    void slot();
    void slot(QAction *action);

private slots:
    void setMovable();
    void isMovable();
    void setAllowedAreas();
    void allowedAreas();
    void isAreaAllowed();
    void setOrientation();
    void orientation();
    void clear();
    void addAction();
    void insertAction();
    void addSeparator();
    void insertSeparator();
    void addWidget();
    void insertWidget();
    void actionGeometry();
    void actionAt();
    void toggleViewAction();
    void iconSize();
    void setIconSize();
    void toolButtonStyle();
    void setToolButtonStyle();
    void actionTriggered();
    void movableChanged();
    void allowedAreasChanged();
    void orientationChanged();
    void iconSizeChanged();
    void toolButtonStyleChanged();
    void actionOwnership();
    void widgetAction();
};


QAction *triggered = 0;

tst_QToolBar::tst_QToolBar()
{
    qRegisterMetaType<QSize>("QSize");
    qRegisterMetaType<Qt::Orientation>("Qt::Orientation");
    qRegisterMetaType<Qt::ToolBarAreas>("Qt::ToolBarAreas");
    qRegisterMetaType<Qt::ToolButtonStyle>("Qt::ToolButtonStyle");
}

void tst_QToolBar::slot()
{ }

void tst_QToolBar::slot(QAction *action)
{ ::triggered = action; }

void tst_QToolBar::setMovable()
{ DEPENDS_ON("isMovable()"); }

void tst_QToolBar::isMovable()
{
#define DO_TEST                                                 \
    do {                                                        \
        QVERIFY(tb.isMovable());                                 \
        tb.setMovable(false);                                   \
        QVERIFY(!tb.isMovable());                                \
        QCOMPARE(spy.count(), 1);                                \
        QCOMPARE(spy.at(0).value(0).toBool(), tb.isMovable());   \
        spy.clear();                                            \
        tb.setMovable(tb.isMovable());                          \
        QCOMPARE(spy.count(), 0);                                \
        spy.clear();                                            \
        tb.setMovable(true);                                    \
        QVERIFY(tb.isMovable());                                 \
        QCOMPARE(spy.count(), 1);                                \
        QCOMPARE(spy.at(0).value(0).toBool(), tb.isMovable());   \
        spy.clear();                                            \
        tb.setMovable(tb.isMovable());                          \
        QCOMPARE(spy.count(), 0);                                \
        spy.clear();                                            \
    } while (false)

    QMainWindow mw;
    QToolBar tb;
    QSignalSpy spy(&tb, SIGNAL(movableChanged(bool)));

    DO_TEST;
    mw.addToolBar(&tb);
    DO_TEST;
    mw.removeToolBar(&tb);
    DO_TEST;
}

void tst_QToolBar::setAllowedAreas()
{ DEPENDS_ON("allowedAreas()"); }

void tst_QToolBar::allowedAreas()
{
    QToolBar tb;

    QSignalSpy spy(&tb, SIGNAL(allowedAreasChanged(Qt::ToolBarAreas)));

    // default
    QCOMPARE((int)tb.allowedAreas(), (int)Qt::AllToolBarAreas);
    QVERIFY(tb.isAreaAllowed(Qt::LeftToolBarArea));
    QVERIFY(tb.isAreaAllowed(Qt::RightToolBarArea));
    QVERIFY(tb.isAreaAllowed(Qt::TopToolBarArea));
    QVERIFY(tb.isAreaAllowed(Qt::BottomToolBarArea));

    // a single dock window area
    tb.setAllowedAreas(Qt::LeftToolBarArea);
    QCOMPARE((int)tb.allowedAreas(), (int)Qt::LeftToolBarArea);
    QVERIFY(tb.isAreaAllowed(Qt::LeftToolBarArea));
    QVERIFY(!tb.isAreaAllowed(Qt::RightToolBarArea));
    QVERIFY(!tb.isAreaAllowed(Qt::TopToolBarArea));
    QVERIFY(!tb.isAreaAllowed(Qt::BottomToolBarArea));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(*static_cast<const Qt::ToolBarAreas *>(spy.at(0).value(0).constData()),
            tb.allowedAreas());
    spy.clear();
    tb.setAllowedAreas(tb.allowedAreas());
    QCOMPARE(spy.count(), 0);

    tb.setAllowedAreas(Qt::RightToolBarArea);
    QCOMPARE((int)tb.allowedAreas(), (int)Qt::RightToolBarArea);
    QVERIFY(!tb.isAreaAllowed(Qt::LeftToolBarArea));
    QVERIFY(tb.isAreaAllowed(Qt::RightToolBarArea));
    QVERIFY(!tb.isAreaAllowed(Qt::TopToolBarArea));
    QVERIFY(!tb.isAreaAllowed(Qt::BottomToolBarArea));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(*static_cast<const Qt::ToolBarAreas *>(spy.at(0).value(0).constData()),
            tb.allowedAreas());
    spy.clear();
    tb.setAllowedAreas(tb.allowedAreas());
    QCOMPARE(spy.count(), 0);

    tb.setAllowedAreas(Qt::TopToolBarArea);
    QCOMPARE((int)tb.allowedAreas(), (int)Qt::TopToolBarArea);
    QVERIFY(!tb.isAreaAllowed(Qt::LeftToolBarArea));
    QVERIFY(!tb.isAreaAllowed(Qt::RightToolBarArea));
    QVERIFY(tb.isAreaAllowed(Qt::TopToolBarArea));
    QVERIFY(!tb.isAreaAllowed(Qt::BottomToolBarArea));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(*static_cast<const Qt::ToolBarAreas *>(spy.at(0).value(0).constData()),
            tb.allowedAreas());
    spy.clear();
    tb.setAllowedAreas(tb.allowedAreas());
    QCOMPARE(spy.count(), 0);

    tb.setAllowedAreas(Qt::BottomToolBarArea);
    QCOMPARE((int)tb.allowedAreas(), (int)Qt::BottomToolBarArea);
    QVERIFY(!tb.isAreaAllowed(Qt::LeftToolBarArea));
    QVERIFY(!tb.isAreaAllowed(Qt::RightToolBarArea));
    QVERIFY(!tb.isAreaAllowed(Qt::TopToolBarArea));
    QVERIFY(tb.isAreaAllowed(Qt::BottomToolBarArea));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(*static_cast<const Qt::ToolBarAreas *>(spy.at(0).value(0).constData()),
            tb.allowedAreas());
    spy.clear();
    tb.setAllowedAreas(tb.allowedAreas());
    QCOMPARE(spy.count(), 0);

    // multiple dock window areas
    tb.setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
    QCOMPARE(tb.allowedAreas(), Qt::TopToolBarArea | Qt::BottomToolBarArea);
    QVERIFY(!tb.isAreaAllowed(Qt::LeftToolBarArea));
    QVERIFY(!tb.isAreaAllowed(Qt::RightToolBarArea));
    QVERIFY(tb.isAreaAllowed(Qt::TopToolBarArea));
    QVERIFY(tb.isAreaAllowed(Qt::BottomToolBarArea));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(*static_cast<const Qt::ToolBarAreas *>(spy.at(0).value(0).constData()),
            tb.allowedAreas());
    spy.clear();
    tb.setAllowedAreas(tb.allowedAreas());
    QCOMPARE(spy.count(), 0);

    tb.setAllowedAreas(Qt::LeftToolBarArea | Qt::RightToolBarArea);
    QCOMPARE(tb.allowedAreas(), Qt::LeftToolBarArea | Qt::RightToolBarArea);
    QVERIFY(tb.isAreaAllowed(Qt::LeftToolBarArea));
    QVERIFY(tb.isAreaAllowed(Qt::RightToolBarArea));
    QVERIFY(!tb.isAreaAllowed(Qt::TopToolBarArea));
    QVERIFY(!tb.isAreaAllowed(Qt::BottomToolBarArea));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(*static_cast<const Qt::ToolBarAreas *>(spy.at(0).value(0).constData()),
            tb.allowedAreas());
    spy.clear();
    tb.setAllowedAreas(tb.allowedAreas());
    QCOMPARE(spy.count(), 0);

    tb.setAllowedAreas(Qt::TopToolBarArea | Qt::LeftToolBarArea);
    QCOMPARE(tb.allowedAreas(), Qt::TopToolBarArea | Qt::LeftToolBarArea);
    QVERIFY(tb.isAreaAllowed(Qt::LeftToolBarArea));
    QVERIFY(!tb.isAreaAllowed(Qt::RightToolBarArea));
    QVERIFY(tb.isAreaAllowed(Qt::TopToolBarArea));
    QVERIFY(!tb.isAreaAllowed(Qt::BottomToolBarArea));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(*static_cast<const Qt::ToolBarAreas *>(spy.at(0).value(0).constData()),
            tb.allowedAreas());
    spy.clear();
    tb.setAllowedAreas(tb.allowedAreas());
    QCOMPARE(spy.count(), 0);

    tb.setAllowedAreas(Qt::BottomToolBarArea | Qt::RightToolBarArea);
    QCOMPARE(tb.allowedAreas(), Qt::BottomToolBarArea | Qt::RightToolBarArea);
    QVERIFY(!tb.isAreaAllowed(Qt::LeftToolBarArea));
    QVERIFY(tb.isAreaAllowed(Qt::RightToolBarArea));
    QVERIFY(!tb.isAreaAllowed(Qt::TopToolBarArea));
    QVERIFY(tb.isAreaAllowed(Qt::BottomToolBarArea));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(*static_cast<const Qt::ToolBarAreas *>(spy.at(0).value(0).constData()),
            tb.allowedAreas());
    spy.clear();
    tb.setAllowedAreas(tb.allowedAreas());
    QCOMPARE(spy.count(), 0);
}

void tst_QToolBar::isAreaAllowed()
{ DEPENDS_ON("allowedAreas()"); }

void tst_QToolBar::setOrientation()
{ DEPENDS_ON("orientation()"); }

void tst_QToolBar::orientation()
{
    QToolBar tb;
    QCOMPARE(tb.orientation(), Qt::Horizontal);

    QSignalSpy spy(&tb, SIGNAL(orientationChanged(Qt::Orientation)));

    tb.setOrientation(Qt::Vertical);
    QCOMPARE(tb.orientation(), Qt::Vertical);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(*static_cast<const Qt::Orientation *>(spy.at(0).value(0).constData()),
            tb.orientation());
    spy.clear();
    tb.setOrientation(tb.orientation());
    QCOMPARE(spy.count(), 0);

    tb.setOrientation(Qt::Horizontal);
    QCOMPARE(tb.orientation(), Qt::Horizontal);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(*static_cast<const Qt::Orientation *>(spy.at(0).value(0).constData()),
            tb.orientation());
    spy.clear();
    tb.setOrientation(tb.orientation());
    QCOMPARE(spy.count(), 0);

    tb.setOrientation(Qt::Vertical);
    QCOMPARE(tb.orientation(), Qt::Vertical);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(*static_cast<const Qt::Orientation *>(spy.at(0).value(0).constData()),
            tb.orientation());
    spy.clear();
    tb.setOrientation(tb.orientation());
    QCOMPARE(spy.count(), 0);

    tb.setOrientation(Qt::Horizontal);
    QCOMPARE(tb.orientation(), Qt::Horizontal);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(*static_cast<const Qt::Orientation *>(spy.at(0).value(0).constData()),
            tb.orientation());
    spy.clear();
    tb.setOrientation(tb.orientation());
    QCOMPARE(spy.count(), 0);

    tb.setOrientation(Qt::Vertical);
    QCOMPARE(tb.orientation(), Qt::Vertical);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(*static_cast<const Qt::Orientation *>(spy.at(0).value(0).constData()),
            tb.orientation());
    spy.clear();
    tb.setOrientation(tb.orientation());
    QCOMPARE(spy.count(), 0);
}

void tst_QToolBar::clear()
{
    DEPENDS_ON("addAction()");
    DEPENDS_ON("insertAction()");
    DEPENDS_ON("addSeparator()");
    DEPENDS_ON("insertSeparator()");
    DEPENDS_ON("addWidget()");
    DEPENDS_ON("insertWidget()");
}

void tst_QToolBar::addAction()
{
    QToolBar tb;

    {
        QAction action(0);

        QCOMPARE(tb.actions().count(), 0);
        tb.addAction(&action);
        QCOMPARE(tb.actions().count(), 1);
        QCOMPARE(tb.actions()[0], &action);

        tb.clear();
        QCOMPARE(tb.actions().count(), 0);
    }

    {
        QString text = "text";
        QPixmap pm(32, 32);
        pm.fill(Qt::blue);
        QIcon icon = pm;

        QAction *action1 = tb.addAction(text);
        QCOMPARE(text, action1->text());

        QAction *action2 = tb.addAction(icon, text);
        QCOMPARE(icon, action2->icon());
        QCOMPARE(text, action2->text());

        QAction *action3 = tb.addAction(text, this, SLOT(slot()));
        QCOMPARE(text, action3->text());

        QAction *action4 = tb.addAction(icon, text, this, SLOT(slot()));
        QCOMPARE(icon, action4->icon());
        QCOMPARE(text, action4->text());

        QCOMPARE(tb.actions().count(), 4);
        QCOMPARE(tb.actions()[0], action1);
        QCOMPARE(tb.actions()[1], action2);
        QCOMPARE(tb.actions()[2], action3);
        QCOMPARE(tb.actions()[3], action4);

        tb.clear();
        QCOMPARE(tb.actions().count(), 0);
    }
}

void tst_QToolBar::insertAction()
{
    QToolBar tb;
    QAction action1(0);
    QAction action2(0);
    QAction action3(0);
    QAction action4(0);

    QCOMPARE(tb.actions().count(), 0);
    tb.insertAction(0, &action1);
    tb.insertAction(&action1, &action2);
    tb.insertAction(&action2, &action3);
    tb.insertAction(&action3, &action4);
    QCOMPARE(tb.actions().count(), 4);
    QCOMPARE(tb.actions()[0], &action4);
    QCOMPARE(tb.actions()[1], &action3);
    QCOMPARE(tb.actions()[2], &action2);
    QCOMPARE(tb.actions()[3], &action1);

    tb.clear();
    QCOMPARE(tb.actions().count(), 0);
}

void tst_QToolBar::addSeparator()
{
    QToolBar tb;

    QAction action1(0);
    QAction action2(0);

    tb.addAction(&action1);
    QAction *sep = tb.addSeparator();
    tb.addAction(&action2);

    QCOMPARE(tb.actions().count(), 3);
    QCOMPARE(tb.actions()[0], &action1);
    QCOMPARE(tb.actions()[1], sep);
    QCOMPARE(tb.actions()[2], &action2);

    tb.clear();
    QCOMPARE(tb.actions().count(), 0);
}

void tst_QToolBar::insertSeparator()
{
    QToolBar tb;

    QAction action1(0);
    QAction action2(0);

    tb.addAction(&action1);
    tb.addAction(&action2);
    QAction *sep = tb.insertSeparator(&action2);

    QCOMPARE(tb.actions().count(), 3);
    QCOMPARE(tb.actions()[0], &action1);
    QCOMPARE(tb.actions()[1], sep);
    QCOMPARE(tb.actions()[2], &action2);

    tb.clear();
    QCOMPARE(tb.actions().count(), 0);
}

void tst_QToolBar::addWidget()
{
    QToolBar tb;
    QWidget w(&tb);

    QAction action1(0);
    QAction action2(0);

    tb.addAction(&action1);
    QAction *widget = tb.addWidget(&w);
    tb.addAction(&action2);

    QCOMPARE(tb.actions().count(), 3);
    QCOMPARE(tb.actions()[0], &action1);
    QCOMPARE(tb.actions()[1], widget);
    QCOMPARE(tb.actions()[2], &action2);

    // it should be possible to reuse the action returned by
    // addWidget() to place the widget somewhere else in the toolbar
    tb.removeAction(widget);
    QCOMPARE(tb.actions().count(), 2);
    QCOMPARE(tb.actions()[0], &action1);
    QCOMPARE(tb.actions()[1], &action2);

    tb.addAction(widget);
    QCOMPARE(tb.actions().count(), 3);
    QCOMPARE(tb.actions()[0], &action1);
    QCOMPARE(tb.actions()[1], &action2);
    QCOMPARE(tb.actions()[2], widget);

    tb.clear();
    QCOMPARE(tb.actions().count(), 0);
}

void tst_QToolBar::insertWidget()
{
    QToolBar tb;
    QWidget w(&tb);

    QAction action1(0);
    QAction action2(0);

    tb.addAction(&action1);
    tb.addAction(&action2);
    QAction *widget = tb.insertWidget(&action2, &w);

    QCOMPARE(tb.actions().count(), 3);
    QCOMPARE(tb.actions()[0], &action1);
    QCOMPARE(tb.actions()[1], widget);
    QCOMPARE(tb.actions()[2], &action2);

    // it should be possible to reuse the action returned by
    // addWidget() to place the widget somewhere else in the toolbar
    tb.removeAction(widget);
    QCOMPARE(tb.actions().count(), 2);
    QCOMPARE(tb.actions()[0], &action1);
    QCOMPARE(tb.actions()[1], &action2);

    tb.insertAction(&action1, widget);
    QCOMPARE(tb.actions().count(), 3);
    QCOMPARE(tb.actions()[0], widget);
    QCOMPARE(tb.actions()[1], &action1);
    QCOMPARE(tb.actions()[2], &action2);

    tb.clear();
    QCOMPARE(tb.actions().count(), 0);
    
    {
        QToolBar tb;
        QPointer<QWidget> widget = new QWidget;
        QAction *action = tb.addWidget(widget);
        QVERIFY(action->parent() == &tb);
        
        QToolBar tb2;
        tb.removeAction(action);
        tb2.addAction(action);
        QVERIFY(widget && widget->parent() == &tb2);
        QVERIFY(action->parent() == &tb2);
    }
}

QT_BEGIN_NAMESPACE
extern void qt_x11_wait_for_window_manager(QWidget* w);
QT_END_NAMESPACE

void tst_QToolBar::actionGeometry()
{
    QToolBar tb;

    QAction action1(0);
    QAction action2(0);
    QAction action3(0);
    QAction action4(0);

    tb.addAction(&action1);
    tb.addAction(&action2);
    tb.addAction(&action3);
    tb.addAction(&action4);

    tb.show();
#ifdef Q_WS_X11
    qt_x11_wait_for_window_manager(&tb);
#endif

    QRect rect1 = tb.actionGeometry(&action1);
    QRect rect2 = tb.actionGeometry(&action2);
    QRect rect3 = tb.actionGeometry(&action3);
    QRect rect4 = tb.actionGeometry(&action4);

    QVERIFY(rect1.isValid());
    QVERIFY(!rect1.isNull());
    QVERIFY(!rect1.isEmpty());

    QVERIFY(rect2.isValid());
    QVERIFY(!rect2.isNull());
    QVERIFY(!rect2.isEmpty());

    QVERIFY(rect3.isValid());
    QVERIFY(!rect3.isNull());
    QVERIFY(!rect3.isEmpty());

    QVERIFY(rect4.isValid());
    QVERIFY(!rect4.isNull());
    QVERIFY(!rect4.isEmpty());

    QCOMPARE(tb.actionAt(rect1.center()), &action1);
    QCOMPARE(tb.actionAt(rect2.center()), &action2);
    QCOMPARE(tb.actionAt(rect3.center()), &action3);
    QCOMPARE(tb.actionAt(rect4.center()), &action4);
}

void tst_QToolBar::actionAt()
{ DEPENDS_ON("actionGeometry()"); }

void tst_QToolBar::toggleViewAction()
{
    {
        QToolBar tb;
        QAction *toggleViewAction = tb.toggleViewAction();
        QVERIFY(tb.isHidden());
        toggleViewAction->trigger();
        QVERIFY(!tb.isHidden());
        toggleViewAction->trigger();
        QVERIFY(tb.isHidden());
    }

    {
        QMainWindow mw;
        QToolBar tb(&mw);
        mw.addToolBar(&tb);
        mw.show();
        QAction *toggleViewAction = tb.toggleViewAction();
        QVERIFY(!tb.isHidden());
        toggleViewAction->trigger();
        QVERIFY(tb.isHidden());
        toggleViewAction->trigger();
        QVERIFY(!tb.isHidden());
        toggleViewAction->trigger();
        QVERIFY(tb.isHidden());
    }
}

void tst_QToolBar::setIconSize()
{ DEPENDS_ON("iconSize()"); }

void tst_QToolBar::iconSize()
{
    {
        QToolBar tb;

        QSignalSpy spy(&tb, SIGNAL(iconSizeChanged(QSize)));

        // the default is determined by the style
        const int metric = tb.style()->pixelMetric(QStyle::PM_ToolBarIconSize);
        const QSize defaultIconSize = QSize(metric, metric);
        const QSize smallIconSize = QSize(metric / 2, metric / 2);
        const QSize largeIconSize = QSize(metric * 2, metric * 2);

        QCOMPARE(tb.iconSize(), defaultIconSize);
        tb.setIconSize(defaultIconSize);
        QCOMPARE(tb.iconSize(), defaultIconSize);
        QCOMPARE(spy.count(), 0);

        spy.clear();
        tb.setIconSize(largeIconSize);
        QCOMPARE(tb.iconSize(), largeIconSize);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.first().first().toSize(), largeIconSize);

        // no-op
        spy.clear();
        tb.setIconSize(largeIconSize);
        QCOMPARE(tb.iconSize(), largeIconSize);
        QCOMPARE(spy.count(), 0);

        spy.clear();
        tb.setIconSize(defaultIconSize);
        QCOMPARE(tb.iconSize(), defaultIconSize);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.first().first().toSize(), defaultIconSize);

        // no-op
        spy.clear();
        tb.setIconSize(defaultIconSize);
        QCOMPARE(tb.iconSize(), defaultIconSize);
        QCOMPARE(spy.count(), 0);

        spy.clear();
        tb.setIconSize(smallIconSize);
        QCOMPARE(tb.iconSize(), smallIconSize);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.first().first().toSize(), smallIconSize);

        // no-op
        spy.clear();
        tb.setIconSize(smallIconSize);
        QCOMPARE(tb.iconSize(), smallIconSize);
        QCOMPARE(spy.count(), 0);

        // setting the icon size to an invalid QSize will reset the
        // iconSize property to the default
        tb.setIconSize(QSize());
        QCOMPARE(tb.iconSize(), defaultIconSize);
        QCOMPARE(spy.size(), 1);
        QCOMPARE(spy.first().first().toSize(), defaultIconSize);
        spy.clear();
    }

    {
        QMainWindow mw;
        QToolBar tb;
        QSignalSpy mwSpy(&mw, SIGNAL(iconSizeChanged(QSize)));
        QSignalSpy tbSpy(&tb, SIGNAL(iconSizeChanged(QSize)));

        // the default is determined by the style
        const int metric = tb.style()->pixelMetric(QStyle::PM_ToolBarIconSize);
        const QSize defaultIconSize = QSize(metric, metric);
        const QSize smallIconSize = QSize(metric / 2, metric / 2);
        const QSize largeIconSize = QSize(metric * 2, metric * 2);

        mw.setIconSize(smallIconSize);

        // explicitly set it to the default
        tb.setIconSize(defaultIconSize);
        QCOMPARE(tb.iconSize(), defaultIconSize);
        QCOMPARE(tbSpy.count(), 0);

        mw.addToolBar(&tb);

        // tb icon size should not change since it has been explicitly set
        QCOMPARE(tb.iconSize(), defaultIconSize);
        QCOMPARE(tbSpy.count(), 0);

        mw.setIconSize(largeIconSize);

        QCOMPARE(tb.iconSize(), defaultIconSize);
        QCOMPARE(tbSpy.count(), 0);

        mw.setIconSize(defaultIconSize);

        QCOMPARE(tb.iconSize(), defaultIconSize);
        QCOMPARE(tbSpy.count(), 0);

        mw.setIconSize(smallIconSize);

        QCOMPARE(tb.iconSize(), defaultIconSize);
        QCOMPARE(tbSpy.count(), 0);

        // resetting to the default should cause the toolbar to take
        // on the mainwindow's icon size
        tb.setIconSize(QSize());
        QCOMPARE(tb.iconSize(), smallIconSize);
        QCOMPARE(tbSpy.size(), 1);
        QCOMPARE(tbSpy.first().first().toSize(), smallIconSize);
        tbSpy.clear();
    }
}

void tst_QToolBar::setToolButtonStyle()
{ DEPENDS_ON("toolButtonStyle()"); }

void tst_QToolBar::toolButtonStyle()
{
    {
        QToolBar tb;

        QSignalSpy spy(&tb, SIGNAL(toolButtonStyleChanged(Qt::ToolButtonStyle)));

        // no-op
        QCOMPARE(tb.toolButtonStyle(), Qt::ToolButtonIconOnly);
        tb.setToolButtonStyle(Qt::ToolButtonIconOnly);
        QCOMPARE(tb.toolButtonStyle(), Qt::ToolButtonIconOnly);
        QCOMPARE(spy.count(), 0);

        tb.setToolButtonStyle(Qt::ToolButtonTextOnly);
        QCOMPARE(tb.toolButtonStyle(), Qt::ToolButtonTextOnly);
        QCOMPARE(spy.count(), 1);
        spy.clear();

        // no-op
        tb.setToolButtonStyle(Qt::ToolButtonTextOnly);
        QCOMPARE(tb.toolButtonStyle(), Qt::ToolButtonTextOnly);
        QCOMPARE(spy.count(), 0);

        tb.setToolButtonStyle(Qt::ToolButtonIconOnly);
        QCOMPARE(tb.toolButtonStyle(), Qt::ToolButtonIconOnly);
        QCOMPARE(spy.count(), 1);
        spy.clear();

        // no-op
        tb.setToolButtonStyle(Qt::ToolButtonIconOnly);
        QCOMPARE(tb.toolButtonStyle(), Qt::ToolButtonIconOnly);
        QCOMPARE(spy.count(), 0);

        tb.setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        QCOMPARE(tb.toolButtonStyle(), Qt::ToolButtonTextBesideIcon);
        QCOMPARE(spy.count(), 1);
        spy.clear();

        // no-op
        tb.setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        QCOMPARE(tb.toolButtonStyle(), Qt::ToolButtonTextBesideIcon);
        QCOMPARE(spy.count(), 0);

        tb.setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        QCOMPARE(tb.toolButtonStyle(), Qt::ToolButtonTextUnderIcon);
        QCOMPARE(spy.count(), 1);
        spy.clear();

        // no-op
        tb.setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        QCOMPARE(tb.toolButtonStyle(), Qt::ToolButtonTextUnderIcon);
        QCOMPARE(spy.count(), 0);
    }

    {
        QMainWindow mw;
        QToolBar tb;
        QSignalSpy mwSpy(&mw, SIGNAL(toolButtonStyleChanged(Qt::ToolButtonStyle)));
        QSignalSpy tbSpy(&tb, SIGNAL(toolButtonStyleChanged(Qt::ToolButtonStyle)));

        mw.setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

        // explicitly set the tb to the default
        tb.setToolButtonStyle(Qt::ToolButtonIconOnly);
        QCOMPARE(tb.toolButtonStyle(), Qt::ToolButtonIconOnly);
        QCOMPARE(tbSpy.count(), 0);

        mw.addToolBar(&tb);

        // tb icon size should not change since it has been explicitly set
        QCOMPARE(tb.toolButtonStyle(), Qt::ToolButtonIconOnly);
        QCOMPARE(tbSpy.count(), 0);

        mw.setToolButtonStyle(Qt::ToolButtonIconOnly);

        QCOMPARE(tb.toolButtonStyle(), Qt::ToolButtonIconOnly);
        QCOMPARE(tbSpy.count(), 0);

        mw.setToolButtonStyle(Qt::ToolButtonTextOnly);

        QCOMPARE(tb.toolButtonStyle(), Qt::ToolButtonIconOnly);
        QCOMPARE(tbSpy.count(), 0);

        mw.setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        QCOMPARE(tb.toolButtonStyle(), Qt::ToolButtonIconOnly);
        QCOMPARE(tbSpy.count(), 0);

        // note: there is no way to clear the explicitly set tool
        // button style... once you explicitly set it, the toolbar
        // will never follow the mainwindow again
    }
}

void tst_QToolBar::actionTriggered()
{
    QToolBar tb;
    connect(&tb, SIGNAL(actionTriggered(QAction *)), SLOT(slot(QAction *)));

    QAction action1(0);
    QAction action2(0);
    QAction action3(0);
    QAction action4(0);

    tb.addAction(&action1);
    tb.addAction(&action2);
    tb.addAction(&action3);
    tb.addAction(&action4);

    tb.show();
#ifdef Q_WS_X11
    qt_x11_wait_for_window_manager(&tb);
#endif

    QRect rect1 = tb.actionGeometry(&action1);
    QRect rect2 = tb.actionGeometry(&action2);
    QRect rect3 = tb.actionGeometry(&action3);
    QRect rect4 = tb.actionGeometry(&action4);
    QAbstractButton *button1 = qobject_cast<QAbstractButton *>(tb.childAt(rect1.center()));
    QAbstractButton *button2 = qobject_cast<QAbstractButton *>(tb.childAt(rect2.center()));
    QAbstractButton *button3 = qobject_cast<QAbstractButton *>(tb.childAt(rect3.center()));
    QAbstractButton *button4 = qobject_cast<QAbstractButton *>(tb.childAt(rect4.center()));
    QVERIFY(button1 != 0);
    QVERIFY(button2 != 0);
    QVERIFY(button3 != 0);
    QVERIFY(button4 != 0);

    ::triggered = 0;
    QTest::mouseClick(button1, Qt::LeftButton);
    QCOMPARE(::triggered, &action1);

    ::triggered = 0;
    QTest::mouseClick(button2, Qt::LeftButton);
    QCOMPARE(::triggered, &action2);

    ::triggered = 0;
    QTest::mouseClick(button3, Qt::LeftButton);
    QCOMPARE(::triggered, &action3);

    ::triggered = 0;
    QTest::mouseClick(button4, Qt::LeftButton);
    QCOMPARE(::triggered, &action4);
}

void tst_QToolBar::movableChanged()
{ DEPENDS_ON("isMovable()"); }

void tst_QToolBar::allowedAreasChanged()
{ DEPENDS_ON("allowedAreas()"); }

void tst_QToolBar::orientationChanged()
{ DEPENDS_ON("orientation()"); }

void tst_QToolBar::iconSizeChanged()
{ DEPENDS_ON("iconSize()"); }

void tst_QToolBar::toolButtonStyleChanged()
{ DEPENDS_ON("toolButtonStyle()"); }

void tst_QToolBar::actionOwnership()
{
    {
        QToolBar *tb1 = new QToolBar;
        QToolBar *tb2 = new QToolBar;
        
        QPointer<QAction> action = tb1->addAction("test");
        QVERIFY(action->parent() == tb1);
        
        tb2->addAction(action);
        QVERIFY(action->parent() == tb1);
        
        delete tb1;
        QVERIFY(!action);
        delete tb2;
    }
    {
        QToolBar *tb1 = new QToolBar;
        QToolBar *tb2 = new QToolBar;
        
        QPointer<QAction> action = tb1->addAction("test");
        QVERIFY(action->parent() == tb1);
        
        tb1->removeAction(action);
        QVERIFY(action->parent() == tb1);
        
        tb2->addAction(action);
        QVERIFY(action->parent() == tb1);
        
        delete tb1;
        QVERIFY(!action);
        delete tb2;
    }
}

void tst_QToolBar::widgetAction()
{
    // ensure that a QWidgetAction without widget behaves like a normal action
    QToolBar tb;
    QWidgetAction *a = new QWidgetAction(0);
    a->setIconText("Blah");

    tb.addAction(a);
    QWidget *w = tb.widgetForAction(a);
    QVERIFY(w);
    QToolButton *button = qobject_cast<QToolButton *>(w);
    QVERIFY(button);
    QCOMPARE(a->iconText(), button->text());

    delete a;
}

QTEST_MAIN(tst_QToolBar)
#include "tst_qtoolbar.moc"

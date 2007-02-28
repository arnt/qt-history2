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
#include <qtoolbar.h>
#include <qcombobox.h>
#if QT_VERSION >= 0x040200
#include <qwidgetaction.h>
#endif

//TESTED_CLASS=
//TESTED_FILES=gui/kernel/qwidgetaction.h gui/kernel/qwidgetaction.cpp

class tst_QWidgetAction : public QObject
{
    Q_OBJECT
private slots:
#if QT_VERSION >= 0x040200
    void defaultWidget();
    void visibilityUpdate();
    void customWidget();
    void keepOwnership();
    void visibility();
#endif
};

#if QT_VERSION >= 0x040200
void tst_QWidgetAction::defaultWidget()
{
    {
        QToolBar tb1;

        QPointer<QComboBox> combo = new QComboBox(&tb1);

        QWidgetAction *action = new QWidgetAction(0);
        action->setDefaultWidget(combo);

        QVERIFY(!combo->isVisible());
        QVERIFY(!combo->parent());
        QVERIFY(action->isVisible());
    
        delete action;
        QVERIFY(!combo);
    }
    {
        QToolBar tb1;

        QPointer<QComboBox> combo = new QComboBox(&tb1);
        combo->hide();

        QWidgetAction *action = new QWidgetAction(0);
        action->setDefaultWidget(combo);

        // explicitly hidden widgets should also set the action invisible
        QVERIFY(!action->isVisible());
        
        delete action;
    }
    {
        QPointer<QComboBox> combo = new QComboBox(0);
        combo->show();

        QWidgetAction *action = new QWidgetAction(0);
        action->setDefaultWidget(combo);

        QVERIFY(action->isVisible());
        QVERIFY(!combo->isVisible());
        
        delete action;
    }
    {
        QToolBar tb1;
        tb1.show();
        QToolBar tb2;
        tb2.show();

        QPointer<QComboBox> combo = new QComboBox(0);

        QWidgetAction *action = new QWidgetAction(0);
        action->setDefaultWidget(combo);
        
        tb1.addAction(action);
        QVERIFY(combo->parent() == &tb1);
        qApp->processEvents();
        QVERIFY(combo->isVisible());

        // not supported, not supposed to work, hence the parent() check
        tb2.addAction(action);
        QVERIFY(combo->parent() == &tb1);
        
        tb2.removeAction(action);
        tb1.removeAction(action);
        
        qApp->processEvents(); //the call to hide is delayd by the toolbar layout
        QVERIFY(!combo->isVisible());
        
        tb2.addAction(action);
        qApp->processEvents(); //the call to hide is delayd by the toolbar layout
        QVERIFY(combo->parent() == &tb2);
        QVERIFY(combo->isVisible());
        
        tb1.addAction(action);
        QVERIFY(combo->parent() == &tb2);
    
        delete action;
        QVERIFY(!combo);
    }
    {
        QWidgetAction *a = new QWidgetAction(0);
        QVERIFY(!a->defaultWidget());

        QPointer<QComboBox> combo1 = new QComboBox;
        a->setDefaultWidget(combo1);
        QVERIFY(a->defaultWidget() == combo1);
        a->setDefaultWidget(combo1);
        QVERIFY(combo1);
        QVERIFY(a->defaultWidget() == combo1);

        QPointer<QComboBox> combo2 = new QComboBox;
        QVERIFY(combo1 != combo2);
        
        a->setDefaultWidget(combo2);
        QVERIFY(!combo1);
        QVERIFY(a->defaultWidget() == combo2);

        delete a;
        QVERIFY(!combo2);
    }
}

void tst_QWidgetAction::visibilityUpdate()
{
    // actually keeping the widget's state in sync with the
    // action in terms of visibility is QToolBar's responsibility.
    QToolBar tb;
    tb.show();
    
    QComboBox *combo = new QComboBox(0);
    QWidgetAction *action = new QWidgetAction(0);
    action->setDefaultWidget(combo);
    
    tb.addAction(action);
    qApp->processEvents(); //the call to show is delayed by the toolbar layout
    QVERIFY(combo->isVisible());
    QVERIFY(action->isVisible());
    
    action->setVisible(false);
    qApp->processEvents(); //the call to hide is delayed by the toolbar layout
    QVERIFY(!combo->isVisible());
    
    delete action;
    // action also deletes combo
}

class ComboAction : public QWidgetAction
{
public:
    inline ComboAction(QObject *parent) : QWidgetAction(parent) {}

    QList<QWidget *> createdWidgets() const { return QWidgetAction::createdWidgets(); }

protected:
    virtual QWidget *createWidget(QWidget *parent);
};

QWidget *ComboAction::createWidget(QWidget *parent)
{
    return new QComboBox(parent);
}

void tst_QWidgetAction::customWidget()
{
    QToolBar tb1;
    tb1.show();
    QToolBar tb2;
    tb2.show();

    ComboAction *action = new ComboAction(0);

    tb1.addAction(action);

    QList<QWidget *> combos = action->createdWidgets();
    QCOMPARE(combos.count(), 1);

    QPointer<QComboBox> combo1 = qobject_cast<QComboBox *>(combos.at(0));
    QVERIFY(combo1);
    
    tb2.addAction(action);
    
    combos = action->createdWidgets();
    QCOMPARE(combos.count(), 2);
    
    QVERIFY(combos.at(0) == combo1);
    QPointer<QComboBox> combo2 = qobject_cast<QComboBox *>(combos.at(1));
    QVERIFY(combo2);
    
    tb2.removeAction(action);
    QVERIFY(combo2);
    // widget is deleted using deleteLater(), so process that posted event
    QCoreApplication::sendPostedEvents(combo2, QEvent::DeferredDelete);
    QVERIFY(!combo2);

    delete action;
    QVERIFY(!combo1);
    QVERIFY(!combo2);
}

void tst_QWidgetAction::keepOwnership()        
{
    QPointer<QComboBox> combo = new QComboBox;
    QWidgetAction *action = new QWidgetAction(0);
    action->setDefaultWidget(combo);
    
    {
        QToolBar *tb = new QToolBar;
        tb->addAction(action);
        QVERIFY(combo->parent() == tb);
        delete tb;
    }
    
    QVERIFY(combo);
    delete action;
    QVERIFY(!combo);
}

void tst_QWidgetAction::visibility()
{
    {
        QWidgetAction *a = new QWidgetAction(0);
        QComboBox *combo = new QComboBox;
        a->setDefaultWidget(combo);
        
        QToolBar *tb = new QToolBar;
        tb->addAction(a);
        QVERIFY(!combo->isVisible());
        tb->show();
        QVERIFY(combo->isVisible());
        
        delete tb;
        
        delete a;
    }
    {
        QWidgetAction *a = new QWidgetAction(0);
        QComboBox *combo = new QComboBox;
        a->setDefaultWidget(combo);
        
        QToolBar *tb = new QToolBar;
        tb->addAction(a);
        QVERIFY(!combo->isVisible());

        QToolBar *tb2 = new QToolBar;
        tb->removeAction(a);
        tb2->addAction(a);
        QVERIFY(!combo->isVisible());
        tb2->show();
        QVERIFY(combo->isVisible());
        
        delete tb;
        delete tb2;
        
        delete a;
    }
}

#endif

QTEST_MAIN(tst_QWidgetAction)
#include "tst_qwidgetaction.moc"

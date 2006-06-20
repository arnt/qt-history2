/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <qabstractitemview.h>
#include <qstandarditemmodel.h>
#include <qapplication.h>
#include <qlistview.h>
#include <qtableview.h>
#include <qtreeview.h>
#include <qheaderview.h>
#include <qitemeditorfactory.h>
#include <qlineedit.h>
#include <qvalidator.h>

// ok evil, but I want to go home, will fix next week
#define protected public
#include <QItemDelegate>

//TESTED_CLASS=
//TESTED_FILES=gui/itemviews/qitemdelegate.h gui/itemviews/qitemdelegate.cpp

class FooValidator : public QValidator
{
    Q_OBJECT
public:
    FooValidator(QObject *parent = 0)
        : QValidator(parent)
        { }
    ~FooValidator()
        { }
    void fixup(QString &input) const
        { input = QLatin1String("foo"); }
    State validate(QString &input, int &) const
        { return (input == QLatin1String("foo")) ? Acceptable : Invalid; }
};

class tst_QItemDelegate : public QObject
{
    Q_OBJECT

public:
    tst_QItemDelegate();
    virtual ~tst_QItemDelegate();

private slots:
    void getSetCheck();
#if QT_VERSION >= 0x040200
    void textRectangle();
    void editorGetsEnterKeyPress();
#endif
};

// Testing get/set functions
void tst_QItemDelegate::getSetCheck()
{
    QItemDelegate obj1;
    // QItemEditorFactory * QItemDelegate::itemEditorFactory()
    // void QItemDelegate::setItemEditorFactory(QItemEditorFactory *)
    QItemEditorFactory *var1 = new QItemEditorFactory;
    obj1.setItemEditorFactory(var1);
    QCOMPARE(var1, obj1.itemEditorFactory());
    obj1.setItemEditorFactory((QItemEditorFactory *)0);
    QCOMPARE((QItemEditorFactory *)0, obj1.itemEditorFactory());
    delete var1;
}

tst_QItemDelegate::tst_QItemDelegate()
{
}

tst_QItemDelegate::~tst_QItemDelegate()
{
}

#if QT_VERSION >= 0x040200
void tst_QItemDelegate::textRectangle()
{
    QItemDelegate qitemdelegate(0);
    QFont font;
    QRect rect;
    QString string;
    QVERIFY(qitemdelegate.textRectangle(0, rect, font, string) != QRect());

    QStyleOptionViewItem option;
    QSize sizeHint = qitemdelegate.sizeHint(option, QModelIndex());
    QVERIFY(sizeHint!= QSize());
}

void tst_QItemDelegate::editorGetsEnterKeyPress()
{
    QListView view;
    QStandardItemModel model;
    model.appendRow(new QStandardItem(QLatin1String("bar")));
    view.setModel(&model);
    view.show();

    view.edit(model.index(0, 0));

    // get the editor and install custom validator on it
    QList<QLineEdit*> lineEditors = view.findChildren<QLineEdit*>();
    Q_ASSERT(lineEditors.count() == 1);
    QLineEdit *editor = lineEditors.at(0);
    editor->setValidator(new FooValidator());

    QTest::keyPress(editor, Qt::Key_Enter);
    QCoreApplication::instance()->processEvents();

    // the line edit should have gotten the Key_Enter and
    // called the validator's fixup() on the input;
    // then the delegate should have committed the result
    QCOMPARE(model.data(model.index(0, 0)).toString(), QLatin1String("foo"));
}

#endif // QT_VERSION

QTEST_MAIN(tst_QItemDelegate)
#include "tst_qitemdelegate.moc"

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
#include <qdatetimeedit.h>
#include <qlistview.h>
#include <qtableview.h>
#include <qtreeview.h>
#include <qheaderview.h>
#include <qitemeditorfactory.h>
#include <qlineedit.h>
#include <qvalidator.h>
#include <qtablewidget.h>
#include <qtreewidget.h>

#include <QItemDelegate>
#include <QAbstractItemDelegate>

Q_DECLARE_METATYPE(QAbstractItemDelegate::EndEditHint)

//TESTED_CLASS=
//TESTED_FILES=gui/itemviews/qitemdelegate.h gui/itemviews/qitemdelegate.cpp


//Begin of class definitions

class TestItemDelegate : public QItemDelegate
{
public:
    TestItemDelegate(QObject *parent = 0) : QItemDelegate(parent) {}
    ~TestItemDelegate() {}

    void drawDisplay(QPainter *painter,
                     const QStyleOptionViewItem &option,
                     const QRect &rect, const QString &text) const
    {
        displayText = text;
        displayFont = option.font;
        QItemDelegate::drawDisplay(painter, option, rect, text);
    }

    void drawDecoration(QPainter *painter,
                        const QStyleOptionViewItem &option,
                        const QRect &rect, const QPixmap &pixmap) const
    {
        decorationPixmap = pixmap;
        decorationRect = rect;
        QItemDelegate::drawDecoration(painter, option, rect, pixmap); 
    }
                        

    inline QRect textRectangle(QPainter * painter, const QRect &rect,
                               const QFont &font, const QString &text) const
    {
        return QItemDelegate::textRectangle(painter, rect, font, text);
    }

    inline void doLayout(const QStyleOptionViewItem &option,
                         QRect *checkRect, QRect *pixmapRect,
                         QRect *textRect, bool hint) const
    {
        QItemDelegate::doLayout(option, checkRect, pixmapRect, textRect, hint);
    }

    inline QRect rect(const QStyleOptionViewItem &option,
                      const QModelIndex &index, int role) const
    {
        return QItemDelegate::rect(option, index, role);
    }

    inline bool eventFilter(QObject *object, QEvent *event)
    {
        return QItemDelegate::eventFilter(object, event);
    }

    // stored values for testing
    mutable QString displayText;
    mutable QFont displayFont;
    mutable QPixmap decorationPixmap;
    mutable QRect decorationRect;
};

class FooValidator : public QValidator
{
    Q_OBJECT
public:
    FooValidator(QObject *parent = 0) : QValidator(parent) {}
    ~FooValidator() {}

    void fixup(QString &input) const { input = QLatin1String("foo"); }

    State validate(QString &input, int &) const {
        return (input == QLatin1String("foo")) ? Acceptable : Invalid;
    }
};

class DummyModel : public QAbstractTableModel
{
public:
    DummyModel() : pixmap(QPixmap(200, 300)),
                   image(QImage(200, 300, QImage::Format_Mono)),
                   icon(QIcon(QPixmap(200, 300))),
                   color(QColor(23, 243, 23)) {}

    ~DummyModel() {}

    int rowCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent);
        return 4;
    }
    
    int columnCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent);
        return 1;
    }

    QVariant data(const QModelIndex& index, int role) const
    {
        Q_UNUSED(role);
        switch (index.row()) {
        case 1: return QVariant(pixmap);
        case 2: return QVariant(image);
        case 3: return QVariant(icon);
        default: break;
        }
        return QVariant(color);
    }

private:
    QPixmap pixmap;
    QImage image;
    QIcon icon;
    QColor color;
};


class tst_QItemDelegate : public QObject
{
    Q_OBJECT

public:
    tst_QItemDelegate();
    virtual ~tst_QItemDelegate();

private slots:
    void getSetCheck();

    void textRectangle_data();
    void textRectangle();
    void sizeHint_data();
    void sizeHint();
    void editorKeyPress_data();
    void editorKeyPress();
    void font_data();
    void font();
    void doLayout_data();
    void doLayout();
    void rect_data();
    void rect();
    void eventFilter_data();
    void eventFilter();
    void dateTimeEditor_data();
    void dateTimeEditor();
    void index_data();
    void index();
    void decoration_data();
    void decoration();

protected:
    void resetRect(QRect &checkRect,
                   QRect &pixmapRect,
                   QRect &textRect);
};


//End of class definitions

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

void tst_QItemDelegate::textRectangle_data()
{
}

void tst_QItemDelegate::textRectangle()
{
    QFont font;
    QRect rect;
    QString text;
    QRect expected;

    // even an empty item has a size
    const int textMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
    QFontMetrics fontMetrics(font);
    expected = QRect(0, 0, 2 * textMargin, fontMetrics.height() - 1); // ### why -1 ?
    

    TestItemDelegate delegate;
    QRect result = delegate.textRectangle(0, rect, font, text);
    QCOMPARE(result, expected);
}

void tst_QItemDelegate::sizeHint_data()
{
}

void tst_QItemDelegate::sizeHint()
{
    QSize expected;
    QModelIndex index;
    QStyleOptionViewItem option;

    const int textMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
    expected = QSize(2 * textMargin, option.fontMetrics.height() - 1); // ### whu -1 ?

    TestItemDelegate delegate;
    QSize result = delegate.sizeHint(option, index);
    QCOMPARE(result, expected);
}

void tst_QItemDelegate::editorKeyPress_data()
{

}

void tst_QItemDelegate::editorKeyPress()
{
    QListView view;
    QStandardItemModel model;
    model.appendRow(new QStandardItem(QLatin1String("bar")));
    view.setModel(&model);
    view.show();

    view.edit(model.index(0, 0));

    // get the editor and install custom validator on it
    QList<QLineEdit*> lineEditors = qFindChildren<QLineEdit *>(&view);
    Q_ASSERT(lineEditors.count() == 1);
    QLineEdit *editor = lineEditors.at(0);
    editor->setValidator(new FooValidator(editor));

    QTest::keyPress(editor, Qt::Key_Enter);
    QCoreApplication::instance()->processEvents();

    // the line edit should have gotten the Key_Enter and
    // called the validator's fixup() on the input;
    // then the delegate should have committed the result
    QCOMPARE(model.data(model.index(0, 0)).toString(), QLatin1String("foo"));
}

void tst_QItemDelegate::font_data()
{

}

void tst_QItemDelegate::font()
{
    QTableWidget table(1, 2);

    QFont font;
    font.setItalic(false);

    QTableWidgetItem *item = new QTableWidgetItem;
    item->setText("Foo");
    item->setFont(font);
    table.setItem(0, 0, item);
    
    item = new QTableWidgetItem;
    item->setText("Bar");
    item->setFont(font);
    table.setItem(0, 1, item);

    font.setItalic(true);
    table.setFont(font);

    TestItemDelegate *delegate = new TestItemDelegate(&table);
    table.setItemDelegate(delegate);

    table.resize(800, 600);
    table.show();

    QApplication::processEvents();

    QCOMPARE(delegate->displayText, QString("Bar"));
    QCOMPARE(delegate->displayFont.italic(), false);
}

void tst_QItemDelegate::resetRect(QRect& checkRect, QRect& pixmapRect,
                                  QRect& textRect)
{
    checkRect = QRect(0,0, 50, 50);
    pixmapRect = QRect(0, 0, 1000, 1000);
    textRect = QRect(0, 0, 400, 400);
}

void tst_QItemDelegate::doLayout_data()
{

}

// ### convert this test to be data driven
//Testing the different QRect created by the doLayout function.
//Tests are made with different values for the QStyleOptionViewItem properties:
//decorationPosition and position.
void tst_QItemDelegate::doLayout()
{
    TestItemDelegate delegate;
    QStyleOptionViewItem option;
    QRect checkRect;
    QRect pixmapRect;
    QRect textRect;

    option.rect = QRect(0, 0, 400, 400);

    int textMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;

    //Subtest with QStyleOptionViewItem::Top
    option.decorationPosition = QStyleOptionViewItem::Top;

    option.direction = Qt::LeftToRight;
    resetRect(checkRect, pixmapRect, textRect);
    delegate.doLayout(option, &checkRect, &pixmapRect, &textRect, true);

    QCOMPARE(checkRect, QRect(0, 0, 50 + 2 * textMargin, 1000));
    QCOMPARE(pixmapRect, QRect( 50 + 2 * textMargin, 0,
                                1000 + 2 * textMargin, 1000 + textMargin ));
    QCOMPARE(textRect, QRect(50 + 2 * textMargin, 1000 + textMargin,
                             1000 + 2 * textMargin, 400 ));

    option.direction = Qt::RightToLeft;
    resetRect(checkRect, pixmapRect, textRect);
    delegate.doLayout(option, &checkRect, &pixmapRect, &textRect, true);


    QCOMPARE(checkRect, QRect(1000 + 2 * textMargin, 0,
                              50 + 2 * textMargin, 1000 ));
    QCOMPARE(pixmapRect, QRect(0, 0, 1000 + 2 * textMargin, 1000
                               + textMargin));
    QCOMPARE(textRect, QRect(0, 1000 + textMargin,
                             1000 + 2 * textMargin, 400));

    //Subtest with QStyleOptionViewItem::Bottom
    option.decorationPosition = QStyleOptionViewItem::Bottom;

    option.direction = Qt::LeftToRight;
    resetRect(checkRect, pixmapRect, textRect);
    delegate.doLayout(option, &checkRect, &pixmapRect, &textRect, true);
    QCOMPARE(checkRect, QRect( 0, 0, 50 + 2 * textMargin, 1000 ));
    QCOMPARE(pixmapRect, QRect( 50 + 2 * textMargin, 400 + textMargin,
                                1000 + 2 * textMargin, 1000 ));
    QCOMPARE(textRect, QRect( 50 + 2 * textMargin, 0,
                              1000 + 2 * textMargin, 400 + textMargin ));

    option.direction = Qt::RightToLeft;
    resetRect(checkRect, pixmapRect, textRect);
    delegate.doLayout(option, &checkRect, &pixmapRect, &textRect, true);
    QCOMPARE(checkRect, QRect( 1000 + 2 * textMargin, 0,
                               50 + 2 * textMargin, 1000 ));
    QCOMPARE(pixmapRect, QRect( 0, 400 + textMargin,
                                1000 + 2 * textMargin, 1000 ));
    QCOMPARE(textRect, QRect( 0, 0, 1000 + 2 * textMargin, 400 + textMargin ));


    //Subtest with QStyleOptionViewItem::Left
    option.decorationPosition = QStyleOptionViewItem::Left;

    option.direction = Qt::LeftToRight;
    resetRect(checkRect, pixmapRect, textRect);
    delegate.doLayout(option, &checkRect, &pixmapRect, &textRect, true);
    QCOMPARE(checkRect, QRect( 0, 0, 50 + 2 * textMargin, 1000 ));
    QCOMPARE(pixmapRect, QRect( 50 + 2 * textMargin, 0,
                                1000 + 2 * textMargin, 1000 ));
    QCOMPARE(textRect, QRect( 1050 + 4 * textMargin, 0,
                              400 + 2 * textMargin, 1000 ));

    option.direction = Qt::RightToLeft;
    resetRect(checkRect, pixmapRect, textRect);
    delegate.doLayout(option, &checkRect, &pixmapRect, &textRect, true);
    QCOMPARE(checkRect, QRect( 1400 + 4 * textMargin, 0,
                               50 + 2 * textMargin, 1000 ));
    QCOMPARE(pixmapRect, QRect( 400 + 2 * textMargin, 0,
                                1000 + 2 * textMargin, 1000 ));
    QCOMPARE(textRect, QRect( 0, 0, 400 + 2 * textMargin, 1000 ));


    //Subtest with QStyleOptionViewItem::Right
    option.decorationPosition = QStyleOptionViewItem::Right;

    option.direction = Qt::LeftToRight;
    resetRect(checkRect, pixmapRect, textRect);
    delegate.doLayout(option, &checkRect, &pixmapRect, &textRect, true);
    QCOMPARE(checkRect, QRect( 0, 0, 50 + 2 * textMargin, 1000 ));
    QCOMPARE(pixmapRect, QRect( 450 + 4 * textMargin, 0,
                                1000 + 2 * textMargin, 1000 ));
    QCOMPARE(textRect, QRect( 50 + 2 * textMargin, 0,
                              400 + 2 * textMargin, 1000 ));

    option.direction = Qt::RightToLeft;
    resetRect(checkRect, pixmapRect, textRect);
    delegate.doLayout(option, &checkRect, &pixmapRect, &textRect, true);
    QCOMPARE(checkRect, QRect( 1400 + 4 * textMargin, 0,
                               50 + 2 * textMargin, 1000 ));
    QCOMPARE(pixmapRect, QRect( 0, 0, 1000 + 2 * textMargin, 1000 ));
    QCOMPARE(textRect, QRect( 1000 + 2 * textMargin, 0,
                              400 + 2 * textMargin, 1000 ));
}

void tst_QItemDelegate::rect_data()
{

}

void tst_QItemDelegate::rect()
{
    QRect result;
    DummyModel model;
    TestItemDelegate delegate;
    QStyleOptionViewItem option;

    option.decorationSize = QSize(200, 300);

    for (int i = 0; i < 4; ++i) {
        result = delegate.rect(option,
                               model.index(0,0),
                               Qt::DisplayRole);
        QCOMPARE(result, QRect(0, 0, 200, 300));
    }
}


void tst_QItemDelegate::eventFilter_data()
{
}

//TODO : Add a test for the keyPress event
//with Qt::Key_Enter and Qt::Key_Return
void tst_QItemDelegate::eventFilter()
{
    TestItemDelegate delegate;
    QWidget widget;
    QEvent *event;

    qRegisterMetaType<QAbstractItemDelegate::EndEditHint>("QAbstractItemDelegate::EndEditHint");

    QSignalSpy commitDataSpy(&delegate, SIGNAL(commitData(QWidget *)));
    QSignalSpy closeEditorSpy(&delegate,
                              SIGNAL(closeEditor(QWidget *,
                                                 QAbstractItemDelegate::EndEditHint)));

    //Subtest KeyPress
    //For each test we send a key event and check if signals were emitted.
    event = new QKeyEvent(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
    QVERIFY(delegate.eventFilter(&widget, event));
    QCOMPARE(closeEditorSpy.count(), 1);
    QCOMPARE(commitDataSpy.count(), 1);
    delete event;

    event = new QKeyEvent(QEvent::KeyPress, Qt::Key_Backtab, Qt::NoModifier);
    QVERIFY(delegate.eventFilter(&widget, event));
    QCOMPARE(closeEditorSpy.count(), 2);
    QCOMPARE(commitDataSpy.count(), 2);
    delete event;

    event = new QKeyEvent(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    QVERIFY(delegate.eventFilter(&widget, event));
    QCOMPARE(closeEditorSpy.count(), 3);
    QCOMPARE(commitDataSpy.count(), 2);
    delete event;

    event = new QKeyEvent(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier);
    QVERIFY(!delegate.eventFilter(&widget, event));
    QCOMPARE(closeEditorSpy.count(), 3);
    QCOMPARE(commitDataSpy.count(), 2);
    delete event;

    //Subtest focusEvent
    event = new QFocusEvent(QEvent::FocusOut);
    QVERIFY(!delegate.eventFilter(&widget, event));
    QCOMPARE(closeEditorSpy.count(), 4);
    QCOMPARE(commitDataSpy.count(), 3);
    delete event;
}

void tst_QItemDelegate::dateTimeEditor_data()
{

}

void tst_QItemDelegate::dateTimeEditor()
{
    QTime time(7, 16, 34);
    QDate date(2006, 10, 31);
    
    QTableWidgetItem *item1 = new QTableWidgetItem;
    item1->setData(Qt::DisplayRole, time);

    QTableWidgetItem *item2 = new QTableWidgetItem;
    item2->setData(Qt::DisplayRole, date);
    
    QTableWidgetItem *item3 = new QTableWidgetItem;
    item3->setData(Qt::DisplayRole, QDateTime(date, time));

    QTableWidget widget(1, 3);
    widget.setItem(0, 0, item1);
    widget.setItem(0, 1, item2);
    widget.setItem(0, 2, item3);
    widget.show();

    widget.editItem(item1);

    QTestEventLoop::instance().enterLoop(1);

    QTimeEdit *timeEditor = widget.findChild<QTimeEdit *>();
    QVERIFY(timeEditor);
    QCOMPARE(timeEditor->time(), time);

    widget.clearFocus();
    widget.setFocus();
    widget.editItem(item2);

    QTestEventLoop::instance().enterLoop(1);

    QDateEdit *dateEditor = widget.findChild<QDateEdit *>();
    QVERIFY(dateEditor);
    QCOMPARE(dateEditor->date(), date);

    widget.clearFocus();
    widget.setFocus();
    widget.editItem(item3);

    QTestEventLoop::instance().enterLoop(1);

    QDateTimeEdit *dateTimeEditor = widget.findChild<QDateTimeEdit *>();
    QVERIFY(dateTimeEditor);
    QCOMPARE(dateTimeEditor->date(), date);
    QCOMPARE(dateTimeEditor->time(), time);
}

class CustomDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    CustomDelegate(QObject *parent = 0) : QItemDelegate(parent) {}
    mutable QLineEdit *lineEdit;
private:
    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem&,
                          const QModelIndex&) const
    {
        lineEdit = new QLineEdit(parent);
        return lineEdit;
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const
    {
        QVERIFY(index.isValid());
        QVERIFY(index.model());
        QVariant value = index.model()->data(index, Qt::DisplayRole);
        QCOMPARE(lineEdit, static_cast<QLineEdit*>(editor));
        lineEdit->setText(value.toString());
    }

    void setModelData(QWidget *editor,
                      QAbstractItemModel *model,
                      const QModelIndex &index) const
    {
        QVERIFY(index.isValid());
        QCOMPARE(index.model(), model);
        QCOMPARE(lineEdit, static_cast<QLineEdit*>(editor));
    }
};

class CustomTreeWidget : public QTreeWidget
{
public:
    void commitData(QWidget * = 0)
    {
        QLineEdit dummy;
        QAbstractItemView::commitData(&dummy);
    }
};

void tst_QItemDelegate::index_data()
{

}

void tst_QItemDelegate::index()
{
    CustomTreeWidget view;
    CustomDelegate delegate;
    view.setItemDelegate(&delegate);
    QTreeWidgetItem *item = new QTreeWidgetItem(&view);
    item->setText(0, QString());
    item->setData(0, Qt::UserRole, QString());
    item->setFlags(item->flags() | Qt::ItemIsEditable);

    // direct commit
    view.commitData();

    // indirect commit
    view.edit(view.indexAt(QPoint(0, 0)));
    QTest::keyClick(delegate.lineEdit, Qt::Key_Return);
    QApplication::processEvents();
}

void tst_QItemDelegate::decoration_data()
{
    
}

void tst_QItemDelegate::decoration()
{
    QSize size(30, 30);
    QPixmap pixmap(size);

    QTableWidget table(1, 2);

    QTableWidgetItem *item = new QTableWidgetItem;
    item->setData(Qt::DecorationRole, pixmap);
    table.setItem(0, 0, item);
    
    item = new QTableWidgetItem;
    item->setData(Qt::DecorationRole, pixmap);
    table.setItem(0, 1, item);

    TestItemDelegate *delegate = new TestItemDelegate(&table);
    table.setItemDelegate(delegate);

    table.resize(800, 600);
    table.show();

    QApplication::processEvents();

    QCOMPARE(delegate->decorationRect.size(), size);
}

QTEST_MAIN(tst_QItemDelegate)
#include "tst_qitemdelegate.moc"

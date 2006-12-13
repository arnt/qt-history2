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

    inline bool editorEvent(QEvent *event,
                            QAbstractItemModel *model,
                            const QStyleOptionViewItem &option,
                            const QModelIndex &index)
    {
        return QItemDelegate::editorEvent(event, model, option, index);
    }

    // stored values for testing
    mutable QString displayText;
    mutable QFont displayFont;
    mutable QPixmap decorationPixmap;
    mutable QRect decorationRect;
};

class TestItemModel : public QAbstractTableModel
{
public:

    enum Roles {
        PixmapTestRole,
        ImageTestRole,
        IconTestRole,
        ColorTestRole
    };

    TestItemModel(const QSize &size) : size(size){}

    ~TestItemModel() {}

    int rowCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent);
        return 1;
    }
    
    int columnCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent);
        return 1;
    }

    QVariant data(const QModelIndex& index, int role) const
    {
        Q_UNUSED(index);
        static QPixmap pixmap(size);
        static QImage image(size, QImage::Format_Mono);
        static QIcon icon(pixmap);
        static QColor color(Qt::green);

        switch (role) {
        case PixmapTestRole: return pixmap;
        case ImageTestRole:  return image;
        case IconTestRole:   return icon;
        case ColorTestRole:  return color;
        default: break;
        }

        return QVariant();
    }

private:
    QSize size;
};

class tst_QItemDelegate : public QObject
{
    Q_OBJECT

public:
    tst_QItemDelegate();
    virtual ~tst_QItemDelegate();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
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
    void eventFilter();
    void dateTimeEditor_data();
    void dateTimeEditor();
    void decoration_data();
    void decoration();
    void editorEvent_data();
    void editorEvent();
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

    QCOMPARE(obj1.hasClipping(), false);
    obj1.setClipping(true);
    QCOMPARE(obj1.hasClipping(), true);
    obj1.setClipping(false);
    QCOMPARE(obj1.hasClipping(), false);
}

tst_QItemDelegate::tst_QItemDelegate()
{
}

tst_QItemDelegate::~tst_QItemDelegate()
{
}

void tst_QItemDelegate::initTestCase()
{
}

void tst_QItemDelegate::cleanupTestCase()
{
}

void tst_QItemDelegate::init()
{
}

void tst_QItemDelegate::cleanup()
{
}

void tst_QItemDelegate::textRectangle_data()
{
    QFont font;
    QFontMetrics fontMetrics(font);
    int pm = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin);
    int margins = 2 * (pm + 1); // margin on each side of the text
    int height = fontMetrics.height() - 1;  // ### -1 ?

    QTest::addColumn<QString>("text");
    QTest::addColumn<QRect>("rect");
    QTest::addColumn<QRect>("expected");

    QTest::newRow("empty") << QString()
                           << QRect()
                           << QRect(0, 0, margins, height);
}

void tst_QItemDelegate::textRectangle()
{
    QFETCH(QString, text);
    QFETCH(QRect, rect);
    QFETCH(QRect, expected);

    QFont font;
    TestItemDelegate delegate;
    QRect result = delegate.textRectangle(0, rect, font, text);

    QCOMPARE(result, expected);
}

void tst_QItemDelegate::sizeHint_data()
{
    QTest::addColumn<QSize>("expected");

    QFont font;
    QFontMetrics fontMetrics(font);
    int m = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
    QTest::newRow("empty")
        << QSize(2 * m, fontMetrics.height() - 1); // ### why -1 ?

}

void tst_QItemDelegate::sizeHint()
{
    QFETCH(QSize, expected);

    QModelIndex index;
    QStyleOptionViewItem option;

    TestItemDelegate delegate;
    QSize result = delegate.sizeHint(option, index);
    QCOMPARE(result, expected);
}

void tst_QItemDelegate::editorKeyPress_data()
{
    QTest::addColumn<QString>("initial");
    QTest::addColumn<QString>("expected");

    QTest::newRow("foo bar")
        << QString("foo")
        << QString("bar");
}

void tst_QItemDelegate::editorKeyPress()
{
    QFETCH(QString, initial);
    QFETCH(QString, expected);

    QStandardItemModel model;
    model.appendRow(new QStandardItem(initial));

    QListView view;    
    view.setModel(&model);
    view.show();

    QModelIndex index = model.index(0, 0);
    view.edit(index);

    QList<QLineEdit*> lineEditors = qFindChildren<QLineEdit *>(&view);
    QCOMPARE(lineEditors.count(), 1);

    QLineEdit *editor = lineEditors.at(0);
    QTest::keyClicks(editor, expected);
    QTest::keyClick(editor, Qt::Key_Enter);
    QApplication::processEvents();

    QCOMPARE(index.data().toString(), expected);
}

void tst_QItemDelegate::font_data()
{
    QTest::addColumn<QString>("itemText");
    QTest::addColumn<QFont>("itemFont");
    QTest::addColumn<QFont>("viewFont");

    QFont itemFont;
    itemFont.setItalic(true);
    QFont viewFont;

    QTest::newRow("foo italic")
        << QString("foo")
        << itemFont
        << viewFont;
}

void tst_QItemDelegate::font()
{
    QFETCH(QString, itemText);
    QFETCH(QFont, itemFont);
    QFETCH(QFont, viewFont);

    QTableWidget table(1, 1);
    table.setFont(viewFont);

    TestItemDelegate *delegate = new TestItemDelegate(&table);
    table.setItemDelegate(delegate);
    table.show();

    QTableWidgetItem *item = new QTableWidgetItem;
    item->setText(itemText);
    item->setFont(itemFont);
    table.setItem(0, 0, item);

    QApplication::processEvents();

    QCOMPARE(delegate->displayText, item->text());
    QCOMPARE(delegate->displayFont, item->font());
}

//Testing the different QRect created by the doLayout function.
//Tests are made with different values for the QStyleOptionViewItem properties:
//decorationPosition and position.

void tst_QItemDelegate::doLayout_data()
{
    QTest::addColumn<int>("position");
    QTest::addColumn<int>("direction");
    QTest::addColumn<bool>("hint");
    QTest::addColumn<QRect>("itemRect");
    QTest::addColumn<QRect>("checkRect");
    QTest::addColumn<QRect>("pixmapRect");
    QTest::addColumn<QRect>("textRect");
    QTest::addColumn<QRect>("expectedCheckRect");
    QTest::addColumn<QRect>("expectedPixmapRect");
    QTest::addColumn<QRect>("expectedTextRect");

    int m = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
    //int item = 400;
    //int check = 50;
    //int pixmap = 1000;
    //int text = 400;

    QTest::newRow("top, left to right, hint")
        << (int)QStyleOptionViewItem::Top
        << (int)Qt::LeftToRight
        << true
        << QRect(0, 0, 400, 400)
        << QRect(0, 0, 50, 50)
        << QRect(0, 0, 1000, 1000)
        << QRect(0, 0, 400, 400)
        << QRect(0, 0, 50 + 2*m, 1000)
        << QRect(50 + 2*m, 0, 1000 + 2*m, 1000 + m)
        << QRect(50 + 2*m, 1000 + m, 1000 + 2*m, 400);
    /*
    QTest::newRow("top, left to right, limited")
        << (int)QStyleOptionViewItem::Top
        << (int)Qt::LeftToRight
        << false
        << QRect(0, 0, 400, 400)
        << QRect(0, 0, 50, 50)
        << QRect(0, 0, 1000, 1000)
        << QRect(0, 0, 400, 400)
        << QRect(m, (400/2) - (50/2), 50, 50)
        << QRect(50 + 2*m, 0, 1000, 1000)
        << QRect(50 + 2*m, 1000 + m, 400 - (50 + 2*m), 400 - 1000 - m);
    */
    QTest::newRow("top, right to left, hint")
        << (int)QStyleOptionViewItem::Top
        << (int)Qt::RightToLeft
        << true
        << QRect(0, 0, 400, 400)
        << QRect(0, 0, 50, 50)
        << QRect(0, 0, 1000, 1000)
        << QRect(0, 0, 400, 400)
        << QRect(1000 + 2 * m, 0, 50 + 2 * m, 1000)
        << QRect(0, 0, 1000 + 2 * m, 1000 + m)
        << QRect(0, 1000 + m, 1000 + 2 * m, 400);

    QTest::newRow("bottom, left to right, hint")
        << (int)QStyleOptionViewItem::Bottom
        << (int)Qt::LeftToRight
        << true
        << QRect(0, 0, 400, 400)
        << QRect(0, 0, 50, 50)
        << QRect(0, 0, 1000, 1000)
        << QRect(0, 0, 400, 400)
        << QRect(0, 0, 50 + 2 * m, 1000)
        << QRect(50 + 2 * m, 400 + m, 1000 + 2 * m, 1000)
        << QRect(50 + 2 * m, 0, 1000 + 2 * m, 400 + m);

    QTest::newRow("bottom, right to left, hint")
        << (int)QStyleOptionViewItem::Bottom
        << (int)Qt::RightToLeft
        << true
        << QRect(0, 0, 400, 400)
        << QRect(0, 0, 50, 50)
        << QRect(0, 0, 1000, 1000)
        << QRect(0, 0, 400, 400)
        << QRect(1000 + 2 * m, 0, 50 + 2 * m, 1000)
        << QRect(0, 400 + m, 1000 + 2 * m, 1000)
        << QRect(0, 0, 1000 + 2 * m, 400 + m);

    QTest::newRow("left, left to right, hint")
        << (int)QStyleOptionViewItem::Left
        << (int)Qt::LeftToRight
        << true
        << QRect(0, 0, 400, 400)
        << QRect(0, 0, 50, 50)
        << QRect(0, 0, 1000, 1000)
        << QRect(0, 0, 400, 400)
        << QRect(0, 0, 50 + 2 * m, 1000)
        << QRect(50 + 2 * m, 0, 1000 + 2 * m, 1000)
        << QRect(1050 + 4 * m, 0, 400 + 2 * m, 1000);

    QTest::newRow("left, right to left, hint")
        << (int)QStyleOptionViewItem::Left
        << (int)Qt::RightToLeft
        << true
        << QRect(0, 0, 400, 400)
        << QRect(0, 0, 50, 50)
        << QRect(0, 0, 1000, 1000)
        << QRect(0, 0, 400, 400)
        << QRect(1400 + 4 * m, 0, 50 + 2 * m, 1000)
        << QRect(400 + 2 * m, 0, 1000 + 2 * m, 1000)
        << QRect(0, 0, 400 + 2 * m, 1000);

    QTest::newRow("right, left to right, hint")
        << (int)QStyleOptionViewItem::Right
        << (int)Qt::LeftToRight
        << true
        << QRect(0, 0, 400, 400)
        << QRect(0, 0, 50, 50)
        << QRect(0, 0, 1000, 1000)
        << QRect(0, 0, 400, 400)
        << QRect(0, 0, 50 + 2 * m, 1000)
        << QRect(450 + 4 * m, 0, 1000 + 2 * m, 1000)
        << QRect(50 + 2 * m, 0, 400 + 2 * m, 1000);

    QTest::newRow("right, right to left, hint")
        << (int)QStyleOptionViewItem::Right
        << (int)Qt::RightToLeft
        << true
        << QRect(0, 0, 400, 400)
        << QRect(0, 0, 50, 50)
        << QRect(0, 0, 1000, 1000)
        << QRect(0, 0, 400, 400)
        << QRect(1400 + 4 * m, 0, 50 + 2 * m, 1000)
        << QRect(0, 0, 1000 + 2 * m, 1000)
        << QRect(1000 + 2 * m, 0, 400 + 2 * m, 1000);
}

void tst_QItemDelegate::doLayout()
{
    QFETCH(int, position);
    QFETCH(int, direction);
    QFETCH(bool, hint);
    QFETCH(QRect, itemRect);
    QFETCH(QRect, checkRect);
    QFETCH(QRect, pixmapRect);
    QFETCH(QRect, textRect);
    QFETCH(QRect, expectedCheckRect);
    QFETCH(QRect, expectedPixmapRect);
    QFETCH(QRect, expectedTextRect);

    TestItemDelegate delegate;
    QStyleOptionViewItem option;

    option.rect = itemRect;
    option.decorationPosition = (QStyleOptionViewItem::Position)position;
    option.direction = (Qt::LayoutDirection)direction;

    delegate.doLayout(option, &checkRect, &pixmapRect, &textRect, hint);

    QCOMPARE(checkRect, expectedCheckRect);
    QCOMPARE(pixmapRect, expectedPixmapRect);
    QCOMPARE(textRect, expectedTextRect);
}

void tst_QItemDelegate::rect_data()
{
    QTest::addColumn<int>("role");
    QTest::addColumn<QSize>("size");
    QTest::addColumn<QRect>("expected");

    QTest::newRow("pixmap")
        << (int)TestItemModel::PixmapTestRole
        << QSize(200, 300)
        << QRect(0, 0, 200, 300);

    QTest::newRow("image")
        << (int)TestItemModel::ImageTestRole
        << QSize(200, 300)
        << QRect(0, 0, 200, 300);

    QTest::newRow("icon")
        << (int)TestItemModel::IconTestRole
        << QSize(200, 300)
        << QRect(0, 0, 200, 300);

    QTest::newRow("color")
        << (int)TestItemModel::ColorTestRole
        << QSize(200, 300)
        << QRect(0, 0, 200, 300);
}

void tst_QItemDelegate::rect()
{
    QFETCH(int, role);
    QFETCH(QSize, size);
    QFETCH(QRect, expected);

    TestItemModel model(size);
    QStyleOptionViewItem option;
    TestItemDelegate delegate;
    option.decorationSize = size;

    QModelIndex index = model.index(0, 0);
    QVERIFY(index.isValid());
    QRect result = delegate.rect(option, index, role);
    QCOMPARE(result, expected);
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
    QTest::addColumn<QTime>("time");
    QTest::addColumn<QDate>("date");

    QTest::newRow("data")
        << QTime(7, 16, 34)
        << QDate(2006, 10, 31);
}

void tst_QItemDelegate::dateTimeEditor()
{
    QFETCH(QTime, time);
    QFETCH(QDate, date);
    
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

void tst_QItemDelegate::decoration_data()
{
    QTest::addColumn<QSize>("size");

    QTest::newRow("30x30") << QSize(30, 30);
}

void tst_QItemDelegate::decoration()
{
    QFETCH(QSize, size);

    QTableWidget table(1, 1);
    TestItemDelegate delegate;
    table.setItemDelegate(&delegate);
    table.show();

    QTableWidgetItem *item = new QTableWidgetItem;
    item->setData(Qt::DecorationRole, QPixmap(size));
    table.setItem(0, 0, item);

    QApplication::processEvents();

    QCOMPARE(delegate.decorationRect.size(), size);
}

void tst_QItemDelegate::editorEvent_data()
{
    QTest::addColumn<QRect>("rect");
    QTest::addColumn<QString>("text");
    QTest::addColumn<int>("checkState");
    QTest::addColumn<int>("flags");
    QTest::addColumn<QPoint>("pos");
    QTest::addColumn<int>("type");
    QTest::addColumn<bool>("edited");
    QTest::addColumn<int>("expectedCheckState");

    QTest::newRow("unchecked, checkable, release")
        << QRect(0, 0, 20, 20)
        << QString("foo")
        << (int)(Qt::Unchecked)
        << (int)(Qt::ItemIsEditable
            |Qt::ItemIsSelectable
            |Qt::ItemIsUserCheckable
            |Qt::ItemIsEnabled
            |Qt::ItemIsDragEnabled
            |Qt::ItemIsDropEnabled)
        << QPoint(3, 10)
        << (int)(QEvent::MouseButtonRelease)
        << true
        << (int)(Qt::Checked);

    QTest::newRow("checked, checkable, release")
        << QRect(0, 0, 20, 20)
        << QString("foo")
        << (int)(Qt::Checked)
        << (int)(Qt::ItemIsEditable
            |Qt::ItemIsSelectable
            |Qt::ItemIsUserCheckable
            |Qt::ItemIsEnabled
            |Qt::ItemIsDragEnabled
            |Qt::ItemIsDropEnabled)
        << QPoint(3, 10)
        << (int)(QEvent::MouseButtonRelease)
        << true
        << (int)(Qt::Unchecked);

    QTest::newRow("unchecked, not checkable, release")
        << QRect(0, 0, 20, 20)
        << QString("foo")
        << (int)(Qt::Unchecked)
        << (int)(Qt::ItemIsEditable
            |Qt::ItemIsSelectable
            |Qt::ItemIsEnabled
            |Qt::ItemIsDragEnabled
            |Qt::ItemIsDropEnabled)
        << QPoint(3, 10)
        << (int)(QEvent::MouseButtonRelease)
        << false
        << (int)(Qt::Unchecked);

    QTest::newRow("unchecked, checkable, release outside")
        << QRect(0, 0, 20, 20)
        << QString("foo")
        << (int)(Qt::Unchecked)
        << (int)(Qt::ItemIsEditable
            |Qt::ItemIsSelectable
            |Qt::ItemIsUserCheckable
            |Qt::ItemIsEnabled
            |Qt::ItemIsDragEnabled
            |Qt::ItemIsDropEnabled)
        << QPoint(17, 10)
        << (int)(QEvent::MouseButtonRelease)
        << false
        << (int)(Qt::Unchecked);

    QTest::newRow("unchecked, checkable, dblclick")
        << QRect(0, 0, 20, 20)
        << QString("foo")
        << (int)(Qt::Unchecked)
        << (int)(Qt::ItemIsEditable
            |Qt::ItemIsSelectable
            |Qt::ItemIsUserCheckable
            |Qt::ItemIsEnabled
            |Qt::ItemIsDragEnabled
            |Qt::ItemIsDropEnabled)
        << QPoint(3, 10)
        << (int)(QEvent::MouseButtonDblClick)
        << true
        << (int)(Qt::Unchecked);
}

void tst_QItemDelegate::editorEvent()
{
    QFETCH(QRect, rect);
    QFETCH(QString, text);
    QFETCH(int, checkState);
    QFETCH(int, flags);
    QFETCH(QPoint, pos);
    QFETCH(int, type);
    QFETCH(bool, edited);
    QFETCH(int, expectedCheckState);
    
    QStandardItemModel model(1, 1);
    QModelIndex index = model.index(0, 0);
    QVERIFY(index.isValid());

    QStandardItem *item = model.itemFromIndex(index);
    item->setText(text);
    item->setCheckState((Qt::CheckState)checkState);
    item->setFlags((Qt::ItemFlags)flags);

    QStyleOptionViewItem option;
    option.rect = rect;

    QEvent *event = new QMouseEvent((QEvent::Type)type,
                                    pos,
                                    Qt::LeftButton,
                                    Qt::LeftButton,
                                    Qt::NoModifier);
    TestItemDelegate delegate;
    bool wasEdited = delegate.editorEvent(event, &model, option, index);
    delete event;

    QCOMPARE(wasEdited, edited);
    QCOMPARE(index.data(Qt::CheckStateRole).toInt(), expectedCheckState);
}

// ### _not_ covered:

// editing with a custom editor factory

// painting when editing
// painting elided text
// painting wrapped text
// painting focus
// painting icon
// painting color
// painting check
// painting selected

// rect for invalid
// rect for pixmap
// rect for image
// rect for icon
// rect for check

QTEST_MAIN(tst_QItemDelegate)
#include "tst_qitemdelegate.moc"

/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qapplication.h>
#include <qtablewidget.h>
#include <qheaderview.h>
#include <qstatusbar.h>
#include <qmainwindow.h>
#include <qmenu.h>
#include <qcolordialog.h>
#include <qfontdialog.h>
#include <qtoolbar.h>
#include <qlineedit.h>
#include <qevent.h>

class SpreadSheetItem : public QTableWidgetItem
{
public:
    SpreadSheetItem();
    SpreadSheetItem(const QString &text);

    QTableWidgetItem *clone() const;

    QVariant data(int role) const;
    void setData(int role, const QVariant &value);
    QVariant display() const;

    inline QString formula() const
        { return QTableWidgetItem::data(Qt::DisplayRole).toString(); }

    QPoint convertCoords(const QString coords) const;

private:
    mutable bool isResolving;
};

SpreadSheetItem::SpreadSheetItem()
    : QTableWidgetItem(), isResolving(false) {}

SpreadSheetItem::SpreadSheetItem(const QString &text)
    : QTableWidgetItem(text), isResolving(false) {}

QTableWidgetItem *SpreadSheetItem::clone() const
{
    SpreadSheetItem *item = new SpreadSheetItem();
    *item = *this;
    return item;
}

QVariant SpreadSheetItem::data(int role) const
{
    if (role == Qt::EditRole || role == Qt::StatusTipRole)
        return formula();

    if (role == Qt::DisplayRole)
        return display();

    QString t = display().toString();
    bool isNumber = false;
    int number = t.toInt(&isNumber);

    if (role == Qt::TextColorRole) {
        if (!isNumber)
            return qVariantFromValue(QColor(Qt::black));
        else if (number < 0)
            return qVariantFromValue(QColor(Qt::red));
        return qVariantFromValue(QColor(Qt::blue));
    }

    if (role == Qt::TextAlignmentRole)
        if (!t.isEmpty() && (t.at(0).isNumber() || t.at(0) == '-'))
            return (int)(Qt::AlignRight | Qt::AlignVCenter);

    return QTableWidgetItem::data(role);
}

void SpreadSheetItem::setData(int role, const QVariant &value)
{
    QTableWidgetItem::setData(role, value);
    if (tableWidget())
        tableWidget()->viewport()->update();
}

QVariant SpreadSheetItem::display() const
{
    // check if the string is actially a formula or not
    QString formula = this->formula();
    QStringList list = formula.split(' ');
    if (list.count() != 3)
        return formula; // its a normal string

    QString op = list.at(0).toLower();
    QPoint one = convertCoords(list.at(1));
    QPoint two = convertCoords(list.at(2));

    const QTableWidgetItem *start = tableWidget()->item(one.y(), one.x());
    QTableWidgetItem *end = tableWidget()->item(two.y(), two.x());

    if (!start || !end)
        return "Error: Item does not exist!";

    // avoid circular dependencies
    if (isResolving)
        return QVariant();
    isResolving = true;

    QVariant result;
    if (op == "sum") {
        int sum = 0;
        for (int r = tableWidget()->row(start); r <= tableWidget()->row(end); ++r) {
            for (int c = tableWidget()->column(start); c <= tableWidget()->column(end); ++c) {
                const QTableWidgetItem *tableItem = tableWidget()->item(r, c);
                if (tableItem && tableItem != this)
                    sum += tableItem->text().toInt();
            }
        }
        result = sum;
    } else if (op == "+") {
        result = (start->text().toInt() + end->text().toInt());
    } else if (op == "-") {
        result = (start->text().toInt() - end->text().toInt());
    } else if (op == "*") {
        result = (start->text().toInt() * end->text().toInt());
    } else if (op == "/") {
        result = (start->text().toInt() / end->text().toInt());
    } else {
        result = "Error: Operation does not exist!";
    }

    isResolving = false;
    return result;
}

QPoint SpreadSheetItem::convertCoords(const QString coords) const
{
    int r = 0;
    int c = coords.at(0).toUpper().toAscii() - 'A';
    for (int i = 1; i < coords.count(); ++i) {
        r *= 10;
        r += coords.at(i).digitValue();
    }
    return QPoint(c, --r);
}

//   Here we subclass QTableWidget to change the selection behavior to only select with the left mouse button
//   (so we keep the selection when opening the context menu on an item).

class SpreadSheetTable : public QTableWidget
{
    Q_OBJECT
public:
    SpreadSheetTable(int rows, int columns, QWidget *parent) :
        QTableWidget(rows, columns, parent) {}

    QItemSelectionModel::SelectionFlags selectionCommand(const QModelIndex &index,
                                                         const QEvent *event) const;
};

QItemSelectionModel::SelectionFlags SpreadSheetTable::selectionCommand(const QModelIndex &index,
                                                                       const QEvent *event) const
{
    const QMouseEvent *me = event && event->type() == QEvent::MouseButtonPress
                            ? static_cast<const QMouseEvent *>(event) : 0;
    if (me && (me->buttons() & Qt::RightButton || me->buttons() & Qt::MidButton))
        return QItemSelectionModel::NoUpdate;
    return QTableWidget::selectionCommand(index, event);
}

class SpreadSheet : public QMainWindow
{
    Q_OBJECT
public:
    SpreadSheet(int rows, int cols, QWidget *parent = 0);

public slots:
    void updateStatus(QTableWidgetItem *item);
    void updateColor(QTableWidgetItem *item);
    void updateLineEdit(QTableWidgetItem *item);
    void returnPressed();
    void selectColor();
    void selectFont();
    void sum();
    void clear();

protected:
    void setupContextMenu();
    void setupContents();

private:
    QToolBar *toolBar;
    QAction *colorAction;
    QAction *fontAction;
    QAction *firstSeparator;
    QAction *sumAction;
    QAction *seccondSeparator;
    QAction *clearAction;
    QTableWidget *table;
    QLineEdit *lineEdit;
};

SpreadSheet::SpreadSheet(int rows, int cols, QWidget *parent)
    : QMainWindow(parent)
{
    addToolBar(toolBar = new QToolBar());

    sumAction = toolBar->addAction(QPixmap(":/images/sum.xpm"), tr("Sum"));
    sumAction->setShortcut(Qt::ALT|Qt::Key_S);
    connect(sumAction, SIGNAL(triggered()), this, SLOT(sum()));

    lineEdit = new QLineEdit();
    toolBar->addWidget(lineEdit);

    firstSeparator = new QAction(toolBar);
    firstSeparator->setSeparator(true);
    toolBar->addAction(firstSeparator);

    fontAction = toolBar->addAction(QPixmap(":/images/font.xpm"), tr("Font..."));
    fontAction->setShortcut(Qt::ALT|Qt::Key_F);
    connect(fontAction, SIGNAL(triggered()), this, SLOT(selectFont()));

    colorAction = toolBar->addAction(QPixmap(16, 16), tr("&Color..."));
    colorAction->setShortcut(Qt::ALT|Qt::Key_C);
    connect(colorAction, SIGNAL(triggered()), this, SLOT(selectColor()));
    updateColor(0);

    seccondSeparator = new QAction(toolBar);
    seccondSeparator->setSeparator(true);
    toolBar->addAction(seccondSeparator);

    clearAction = toolBar->addAction(QPixmap(":/images/clear.xpm"), tr("Clear"));
    clearAction->setShortcut(Qt::Key_Delete);
    connect(clearAction, SIGNAL(triggered()), this, SLOT(clear()));

    table = new SpreadSheetTable(rows, cols, this);
    for (int c = 0; c < cols; ++c) {
        QString character(QChar('A' + c));
        table->setHorizontalHeaderItem(c, new QTableWidgetItem(character));
        table->horizontalHeaderItem(c)->setTextAlignment(Qt::AlignCenter);
    }
    table->setItemPrototype(table->item(rows - 1, cols - 1));

    setupContextMenu();
    setupContents();
    setCentralWidget(table);

    statusBar();
    connect(table, SIGNAL(currentItemChanged(QTableWidgetItem*, QTableWidgetItem*)),
            this, SLOT(updateStatus(QTableWidgetItem*)));
    connect(table, SIGNAL(currentItemChanged(QTableWidgetItem*, QTableWidgetItem*)),
            this, SLOT(updateColor(QTableWidgetItem*)));
    connect(table, SIGNAL(currentItemChanged(QTableWidgetItem*, QTableWidgetItem*)),
            this, SLOT(updateLineEdit(QTableWidgetItem*)));
    connect(table, SIGNAL(itemChanged(QTableWidgetItem*)),
            this, SLOT(updateStatus(QTableWidgetItem*)));
    connect(lineEdit, SIGNAL(returnPressed()), this, SLOT(returnPressed()));
    connect(table, SIGNAL(itemChanged(QTableWidgetItem*)),
            this, SLOT(updateLineEdit(QTableWidgetItem*)));
}

void SpreadSheet::updateStatus(QTableWidgetItem *item)
{
    if (item && item == table->currentItem())
        statusBar()->showMessage(item->data(Qt::StatusTipRole).toString(), 1000);
}

void SpreadSheet::updateColor(QTableWidgetItem *item)
{
    QPixmap pix(16, 16);
    QColor col;
    if (item)
        col = item->backgroundColor();
    if (!col.isValid())
        col = palette().base().color();
    pix.fill(col);
    colorAction->setIcon(pix);
}

void SpreadSheet::updateLineEdit(QTableWidgetItem *item)
{
    if (item != table->currentItem())
        return;
    if (item)
        lineEdit->setText(item->data(Qt::EditRole).toString());
    else
        lineEdit->clear();
}

void SpreadSheet::returnPressed()
{
    QString text = lineEdit->text();
    int row = table->currentRow();
    int col = table->currentColumn();
    QTableWidgetItem *item = table->item(row, col);
    if (!item)
        table->setItem(row, col, new SpreadSheetItem(text));
    else
        item->setData(Qt::EditRole, text);
    table->viewport()->update();
}

void SpreadSheet::selectColor()
{
    QTableWidgetItem *item = table->currentItem();
    QColor col = item ? item->backgroundColor() : table->palette().base().color();
    col = QColorDialog::getColor(col, this);
    if (!col.isValid())
        return;

    QList<QTableWidgetItem*> selected = table->selectedItems();
    if (selected.count() == 0)
        return;

    foreach(QTableWidgetItem *i, selected)
        if (i) i->setBackgroundColor(col);

    updateColor(table->currentItem());
}

void SpreadSheet::selectFont()
{
    QList<QTableWidgetItem*> selected = table->selectedItems();
    if (selected.count() == 0)
        return;
    bool ok = false;
    QFont fnt = QFontDialog::getFont(&ok, font(), this);
    if (!ok)
        return;
    foreach(QTableWidgetItem *i, selected)
        if (i) i->setFont(fnt);
}

void SpreadSheet::sum()
{
    QList<QTableWidgetItem*> selected = table->selectedItems();
    if (selected.isEmpty())
        return;
    QTableWidgetItem *first = selected.first();
    QTableWidgetItem *last = selected.last();
    QTableWidgetItem *current = table->currentItem();
    if (!current)
        table->setItem(table->currentRow(), table->currentColumn(),
                       current = new SpreadSheetItem());
    current->setText(QString("sum %1%2 %3%4").
                     arg(QChar('A' + (table->column(first)))).
                     arg((table->row(first) + 1)).
                     arg(QChar('A' + (table->column(last)))).
                     arg((table->row(last) + 1)));
}

void SpreadSheet::clear()
{
    foreach (QTableWidgetItem *i, table->selectedItems())
        delete i;
}

void SpreadSheet::setupContextMenu()
{
    addAction(sumAction);
    addAction(firstSeparator);
    addAction(colorAction);
    addAction(fontAction);
    addAction(seccondSeparator);
    addAction(clearAction);
    setContextMenuPolicy(Qt::ActionsContextMenu);
}

void SpreadSheet::setupContents()
{
    // column 0
    table->setItem(0, 0, new SpreadSheetItem("Item"));
    table->item(0, 0)->setBackgroundColor(Qt::yellow);
    table->item(0, 0)->setToolTip("This column shows the purchased item/service");
    table->setItem(1, 0, new SpreadSheetItem("AirportBus"));
    table->setItem(2, 0, new SpreadSheetItem("Flight (Munich)"));
    table->setItem(3, 0, new SpreadSheetItem("Lunch"));
    table->setItem(4, 0, new SpreadSheetItem("Flight (LA)"));
    table->setItem(5, 0, new SpreadSheetItem("Taxi"));
    table->setItem(6, 0, new SpreadSheetItem("Dinner"));
    table->setItem(7, 0, new SpreadSheetItem("Hotel"));
    table->setItem(8, 0, new SpreadSheetItem("Flight (Oslo)"));
    table->setItem(9, 0, new SpreadSheetItem("Total:"));
    table->item(9,0)->setBackgroundColor(Qt::lightGray);
    // column 1
    table->setItem(0, 1, new SpreadSheetItem("Price"));
    table->item(0, 1)->setBackgroundColor(Qt::yellow);
    table->item(0, 1)->setToolTip("This column shows the price of the purchase");
    table->setItem(1, 1, new SpreadSheetItem("150"));
    table->setItem(2, 1, new SpreadSheetItem("2350"));
    table->setItem(3, 1, new SpreadSheetItem("-14"));
    table->setItem(4, 1, new SpreadSheetItem("980"));
    table->setItem(5, 1, new SpreadSheetItem("5"));
    table->setItem(6, 1, new SpreadSheetItem("120"));
    table->setItem(7, 1, new SpreadSheetItem("300"));
    table->setItem(8, 1, new SpreadSheetItem("1240"));
    table->setItem(9, 1, new SpreadSheetItem());
    table->item(9,1)->setBackgroundColor(Qt::lightGray);
    // column 2
    table->setItem(0, 2, new SpreadSheetItem("Currency"));
    table->item(0,2)->setBackgroundColor(Qt::yellow);
    table->item(0,2)->setToolTip("This column shows the currency");
    table->setItem(1, 2, new SpreadSheetItem("NOK"));
    table->setItem(2, 2, new SpreadSheetItem("NOK"));
    table->setItem(3, 2, new SpreadSheetItem("EUR"));
    table->setItem(4, 2, new SpreadSheetItem("EUR"));
    table->setItem(5, 2, new SpreadSheetItem("USD"));
    table->setItem(6, 2, new SpreadSheetItem("USD"));
    table->setItem(7, 2, new SpreadSheetItem("USD"));
    table->setItem(8, 2, new SpreadSheetItem("USD"));
    table->setItem(9, 2, new SpreadSheetItem());
    table->item(9,2)->setBackgroundColor(Qt::lightGray);
    // column 3
    table->setItem(0, 3, new SpreadSheetItem("Ex.Rate"));
    table->item(0,3)->setBackgroundColor(Qt::yellow);
    table->item(0,3)->setToolTip("This column shows the exchange rate to NOK");
    table->setItem(1, 3, new SpreadSheetItem("1"));
    table->setItem(2, 3, new SpreadSheetItem("1"));
    table->setItem(3, 3, new SpreadSheetItem("8"));
    table->setItem(4, 3, new SpreadSheetItem("8"));
    table->setItem(5, 3, new SpreadSheetItem("7"));
    table->setItem(6, 3, new SpreadSheetItem("7"));
    table->setItem(7, 3, new SpreadSheetItem("7"));
    table->setItem(8, 3, new SpreadSheetItem("7"));
    table->setItem(9, 3, new SpreadSheetItem());
    table->item(9,3)->setBackgroundColor(Qt::lightGray);
    // column 4
    table->setItem(0, 4, new SpreadSheetItem("NOK"));
    table->item(0,4)->setBackgroundColor(Qt::yellow);
    table->item(0,4)->setToolTip("This column shows the expenses in NOK");
    table->setItem(1, 4, new SpreadSheetItem("* B2 D2"));
    table->setItem(2, 4, new SpreadSheetItem("* B3 D3"));
    table->setItem(3, 4, new SpreadSheetItem("* B4 D4"));
    table->setItem(4, 4, new SpreadSheetItem("* B5 D5"));
    table->setItem(5, 4, new SpreadSheetItem("* B6 D6"));
    table->setItem(6, 4, new SpreadSheetItem("* B7 D7"));
    table->setItem(7, 4, new SpreadSheetItem("* B8 D8"));
    table->setItem(8, 4, new SpreadSheetItem("* B9 D9"));
    table->setItem(9, 4, new SpreadSheetItem("sum E2 E9"));
    table->item(9,4)->setBackgroundColor(Qt::lightGray);
}

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    SpreadSheet sheet(100, 26);
    sheet.setWindowIcon(QPixmap(":/images/interview.png"));
    sheet.show();
    sheet.resize(600, 400);
    return app.exec();
}

#include "main.moc"

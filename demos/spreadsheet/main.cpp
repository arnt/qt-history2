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

class SpreadSheetTable : public QTableWidget
{
    Q_OBJECT
public:
    SpreadSheetTable(int rows, int columns, QWidget *parent) :
        QTableWidget(rows, columns, parent) {}

    QItemSelectionModel::SelectionFlags selectionCommand(Qt::ButtonState state,
                                                         const QModelIndex &index,
                                                         QEvent::Type type,
                                                         Qt::Key key) const {
        if (state & Qt::RightButton || state & Qt::MidButton)
            return QItemSelectionModel::NoUpdate;
        return QTableWidget::selectionCommand(state, index, type, key);
    }
};

class SpreadSheetItem : public QTableWidgetItem
{
public:
    SpreadSheetItem(QTableWidget *t) : table(t) {}

    QVariant display() const {
        QStringList list = formula.split(" ");
        if (list.count() < 3)
            return formula;

        QString op = list.at(0).toLower();
        int c1 = list.at(1).at(0).toUpper().ascii() - 'A';
        int r1 = list.at(1).at(1).digitValue() - 1;
        int c2 = list.at(2).at(0).toUpper().ascii() - 'A';
        int r2 = list.at(2).at(1).digitValue() - 1;
        QTableWidgetItem *start = table->item(r1, c1);
        QTableWidgetItem *end = table->item(r2, c2);

        if (!start || !end)
            return "Error: Item does not exist!";

        if (op == "sum") {
            int sum = 0;
            for (int r=table->row(start); r<=table->row(end);  ++r)
                for (int c=table->column(start); c<=table->column(end); ++c)
                    if (table->item(r,c) != this)
                        sum += table->item(r, c)->text().toInt();
            return (sum);
        } else if (op == "+") {
            return (start->text().toInt() + end->text().toInt());
        } else if (op == "-") {
            return (start->text().toInt() - end->text().toInt());
        } else if (op == "*") {
            return (start->text().toInt() * end->text().toInt());
        } else if (op == "/") {
            return (start->text().toInt() / end->text().toInt());
        } else {
            return "Error: Operaion does not exist!";
        }
        return QVariant::Invalid;
    }

    QVariant data(int role) const {
        if (role == QAbstractItemModel::DisplayRole)
            return display();
        if (role == QAbstractItemModel::EditRole || role == QAbstractItemModel::StatusTipRole)
            return formula;
        if (role == QAbstractItemModel::ToolTipRole) {
            switch (table->column(this)) {
            case 0:
                return "This column shows the purchased item/service";
                break;
            case 1:
                return "This column shows the price of the purchase";
                break;
            case 2:
                return "This column shows the currency";
                break;
            case 3:
                return "This column shows the exchange rate to NOK";
                break;
            case 4:
                return "This column shows the expenses in NOK";
                break;
            default:
                return "Demo of a travel expense spreadsheet application";
            }
        }

        QString t = text();
        bool numberOk;
        int number = t.toInt(&numberOk);

        // text color
        if (role == QAbstractItemModel::TextColorRole) {
            if (!numberOk)
                return QColor(Qt::black);
            else if (number < 0)
                return QColor(Qt::red);
            return QColor(Qt::blue);
        }

        // text alignment
        if (role == QAbstractItemModel::TextAlignmentRole) {
            if (!t.isEmpty() && (t.at(0).isNumber() || t.at(0) == '-'))
                return (int)(Qt::AlignRight | Qt::AlignVCenter);
        }

        // font
        if (role == QAbstractItemModel::FontRole) {
            if (numberOk)
                return QFont("Courier", 14, QFont::Bold);
            return QFont("Arial", 14, QFont::Normal);
        }

        // background color
        if (role == QAbstractItemModel::BackgroundColorRole && formula.startsWith("sum")) {
                return QColor(Qt::lightGray);
        }

        return QTableWidgetItem::data(role);
    }

    void setData(int role, const QVariant &value) {
        if (role == QAbstractItemModel::EditRole || role == QAbstractItemModel::DisplayRole) {
            formula = value.toString();
            table->viewport()->update();
            return;
        }
        QTableWidgetItem::setData(role, value);
    }

private:
    QString formula;
    QTableWidget *table;
};


class SpreadSheet : public QMainWindow
{
    Q_OBJECT
public:
    static const int numRows = 20;
    static const int numColumns = 10;

    SpreadSheet(QWidget *parent = 0) : QMainWindow(parent) {
        table = new SpreadSheetTable(numRows, numColumns, this);
        for (int r=0; r<numRows; ++r) {
            for (int c=0; c<numColumns; ++c) {
                QString character(QChar('A' + c));
                QTableWidgetItem *headerItem = new QTableWidgetItem(character);
                headerItem->setTextAlignment(Qt::AlignCenter);
                table->setHorizontalHeaderItem(c, headerItem);
                table->setItem(r, c, new SpreadSheetItem(table));
            }
        }
        //table->verticalHeader()->setResizeMode(QHeaderView::Stretch);
        //table->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
        // column 0
        //table->horizontalHeader()->setResizeMode(QHeaderView::Custom, 0);
        table->item(0,0)->setText("Item");
        table->item(0,0)->setBackgroundColor(Qt::yellow);
        table->item(1,0)->setText("AirportBus");
        table->item(2,0)->setText("Flight (Munich)");
        table->item(3,0)->setText("Lunch");
        table->item(4,0)->setText("Flight (LA)");
        table->item(5,0)->setText("Taxi");
        table->item(6,0)->setText("Dinner");
        table->item(7,0)->setText("Hotel");
        table->item(8,0)->setText("Flight (Oslo)");
        table->item(9,0)->setText("Total:");
        // column 1
        table->item(0,1)->setText("Price");
        table->item(0,1)->setBackgroundColor(Qt::yellow);
        table->item(1,1)->setText("150");
        table->item(2,1)->setText("2350");
        table->item(3,1)->setText("-14");
        table->item(4,1)->setText("980");
        table->item(5,1)->setText("5");
        table->item(6,1)->setText("120");
        table->item(7,1)->setText("300");
        table->item(8,1)->setText("1240");
        // column 2
        table->item(0,2)->setText("Currency");
        table->item(0,2)->setBackgroundColor(Qt::yellow);
        table->item(1,2)->setText("NOK");
        table->item(2,2)->setText("NOK");
        table->item(3,2)->setText("EUR");
        table->item(4,2)->setText("EUR");
        table->item(5,2)->setText("USD");
        table->item(6,2)->setText("USD");
        table->item(7,2)->setText("USD");
        table->item(8,2)->setText("USD");
        // column 3
        table->item(0,3)->setText("Ex.Rate");
        table->item(0,3)->setBackgroundColor(Qt::yellow);
        table->item(1,3)->setText("1");
        table->item(2,3)->setText("1");
        table->item(3,3)->setText("8");
        table->item(4,3)->setText("8");
        table->item(5,3)->setText("7");
        table->item(6,3)->setText("7");
        table->item(7,3)->setText("7");
        table->item(8,3)->setText("7");
        // column 4
        table->item(0,4)->setText("NOK");
        table->item(0,4)->setBackgroundColor(Qt::yellow);
        table->item(1,4)->setText("* B2 D2");
        table->item(2,4)->setText("* B3 D3");
        table->item(3,4)->setText("* B4 D4");
        table->item(4,4)->setText("* B5 D5");
        table->item(5,4)->setText("* B6 D6");
        table->item(6,4)->setText("* B7 D7");
        table->item(7,4)->setText("* B8 D8");
        table->item(8,4)->setText("* B9 D9");
        table->item(9,4)->setText("sum E2 E9");

        setCentralWidget(table);
        statusBar();
        connect(table, SIGNAL(currentChanged(QTableWidgetItem*, QTableWidgetItem*)),
                this, SLOT(updateStatus(QTableWidgetItem*)));
        connect(table, SIGNAL(itemChanged(QTableWidgetItem*)),
                this, SLOT(updateStatus(QTableWidgetItem*)));
        connect(table, SIGNAL(aboutToShowContextMenu(QMenu*, QTableWidgetItem*)),
                this, SLOT(contextActions(QMenu*, QTableWidgetItem*)));
    }

public slots:
    void updateStatus(QTableWidgetItem *item) {
        if (item && item == table->currentItem())
            statusBar()->message(item->data(QAbstractItemModel::StatusTipRole).toString(), 1000);
    }

    void contextActions(QMenu *menu, QTableWidgetItem *item) {
        if (table->selectedItems().count() > 0) {
            connect(menu->addAction("Sum"), SIGNAL(triggered()), this, SLOT(sum()));
            menu->addSeparator();
            connect(menu->addAction("Clear"), SIGNAL(triggered()), this, SLOT(clear()));
        }
        contextItem = item;
    }

    void sum() {
        QList<QTableWidgetItem*> selected = table->selectedItems();
        QTableWidgetItem *first = selected.first();
        QTableWidgetItem *last = selected.last();
        if (first && last && contextItem)
            contextItem->setText(QString("sum %1%2 %3%4").
                                 arg(QChar('A' + (table->column(first)))).
                                 arg((table->row(first) + 1)).
                                 arg(QChar('A' + (table->column(last)))).
                                 arg((table->row(last) + 1)));
    }

    void clear() {
        QList<QTableWidgetItem*> selected = table->selectedItems();
        foreach (QTableWidgetItem *i, selected)
            i->setText(QString::null);
    }

private:
    QTableWidget *table;
    QTableWidgetItem *contextItem;
};


int main(int argc, char** argv) {
    QApplication app(argc, argv);
    SpreadSheet sheet;
    app.setMainWidget(&sheet);
    sheet.show();
    sheet.resize(600, 350);
    return app.exec();
}

#include "main.moc"

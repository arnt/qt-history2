#include <qapplication.h>
#include <qtablewidget.h>
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
        int c1 = list.at(1).at(0).ascii() - 'A';
        int r1 = list.at(1).at(1).digitValue() - 1;
        int c2 = list.at(2).at(0).ascii() - 'A';
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
    static const int numRows = 10;
    static const int numColumns = 5;

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
        else
            qDebug("no item!");
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
    return app.exec();
}

#include "main.moc"

/****************************************************************************
 **
 ** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
 **
 ** This file is an example program for the Qt SQL module.
 ** EDITIONS: NOLIMITS
 **
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 ****************************************************************************/

#include <qapplication.h>
#include <qsplitter.h>
#include <qtableview.h>
#include <qheaderview.h>
#include <qitemdelegate.h>
#include <qpainter.h>

#include "plasmamodel.h"
#include "plasmadelegate.h"
#include "colorfilter.h"

class HexDelegate : public QItemDelegate
{
public:
    HexDelegate(QObject *parent) 
        : QItemDelegate(parent)
    {
        sz = QSize(2, 2);
        textHex = "0x";
        textHex.reserve(8); // fine tuning
    }

    ~HexDelegate()
    {
    }

    QSize sizeHint(const QStyleOptionViewItem &, const QAbstractItemModel *,
                   const QModelIndex &) const;

    void paint(QPainter *, const QStyleOptionViewItem &, const QAbstractItemModel *,
               const QModelIndex &) const;

private:
    mutable QString textHex;
    QPoint pt;
    QSize sz;
};

QSize HexDelegate::sizeHint(const QStyleOptionViewItem &opt,
                            const QAbstractItemModel *,
                            const QModelIndex &) const
{
    static QString textSize("0xFFFFFFFF");
    return QFontMetrics(opt.font).size(0, textSize) + sz;
}

void HexDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                        const QAbstractItemModel *model, const QModelIndex &index) const
{
    static QRect emptyRect;

    textHex.resize(2);
    textHex += QString::number(
                model->data(index, QAbstractItemModel::DisplayRole).toInt(), 16).toUpper();

    // Layout text
    QRect textRect(pt, painter->fontMetrics().size(0, textHex) + sz);
    doLayout(option, &emptyRect, &textRect, false);

    // draw the item
    drawDisplay(painter, option, textRect, textHex);
}


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QSplitter splitter;

    int rc = 100;
    int cc = 160;
 
    QAbstractItemModel *data = new PlasmaModel(rc, cc, &splitter);
    QItemSelectionModel *selections = new QItemSelectionModel(data, data);

    // 1st view
    QTableView *plasmaView = new QTableView(&splitter);
    QAbstractItemDelegate *delegate = new PlasmaDelegate(plasmaView);
    plasmaView->setModel(data);
    plasmaView->setItemDelegate(delegate);
    plasmaView->setSelectionModel(selections);
    plasmaView->setShowGrid(false);
    plasmaView->horizontalHeader()->hide();
    plasmaView->verticalHeader()->hide();

    // 2nd view
    QTableView *hexView = new QTableView(&splitter);
    hexView->setModel(data);
    hexView->setItemDelegate(new HexDelegate(hexView));
    hexView->setSelectionModel(selections);

    for (int c = 0; c < cc; ++c) {
        plasmaView->resizeColumnToContents(c);
        hexView->resizeColumnToContents(c);
    }

    for (int r = 0; r < rc; ++r) {
        plasmaView->resizeRowToContents(r);
        hexView->resizeRowToContents(r);
    }

    app.setMainWidget(&splitter);
    splitter.show();

    return app.exec();
}

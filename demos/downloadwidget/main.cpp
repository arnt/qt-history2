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
#include <qdatetime.h>
#include <qheaderview.h>
#include <qtreewidget.h>
#include <qabstractitemmodel.h>
#include <qitemdelegate.h>
#include <qpainter.h>

class DownloadDelegate : public QItemDelegate
{
public:
    DownloadDelegate(QObject *parent);
    ~DownloadDelegate();

    enum Roles {
        CheckedRole = QAbstractItemModel::DecorationRole,
        DateRole = QAbstractItemModel::DisplayRole,
        ProgressRole = QAbstractItemModel::DisplayRole,
        RatingRole = QAbstractItemModel::DisplayRole
    };  
    
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QAbstractItemModel *model, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QAbstractItemModel *model,
                   const QModelIndex &index) const;
    bool editorEvent(QEvent *e, QAbstractItemModel* model, const QModelIndex &index);

private:
    QPixmap star;
};

DownloadDelegate::DownloadDelegate(QObject *parent)
    : QItemDelegate(parent), star(QPixmap("star.png"))
{
}

DownloadDelegate::~DownloadDelegate()
{
}

void DownloadDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                             const QAbstractItemModel *model, const QModelIndex &index) const
{
    if (index.column() < 2 || index.column() > 3) {
        QItemDelegate::paint(painter, option, model, index);
    } else {
        QPalette::ColorGroup cg = option.state & QStyle::Style_Enabled
                                  ? QPalette::Normal : QPalette::Disabled;
        if (option.state & QStyle::Style_Selected)
            painter->fillRect(option.rect, option.palette.color(cg, QPalette::Highlight));
        if (index.column() == 2) {
            QRect rect(option.rect.x() + 3, option.rect.y() + 3,
                       option.rect.width() - 6, option.rect.height() - 6);
            double download = model->data(index, DownloadDelegate::ProgressRole).toDouble();
            int width = qMin(rect.width(), static_cast<int>(rect.width() * download));
            painter->fillRect(rect.x(), rect.y(), width, rect.height(), Qt::blue);
            painter->fillRect(rect.x() + width, rect.y(), rect.width() - width, rect.height(),
                              option.palette.base());
            painter->drawRect(rect);
        } else if (index.column() == 3) {
            int rating = model->data(index, DownloadDelegate::RatingRole).toInt();
            int width = star.width();
            int x = option.rect.x();
            int y = option.rect.y();
            for (int i = 0; i < rating; ++i) {
                painter->drawPixmap(x, y, star);
                x += width;
            }
        }
    }

    QPen pen = painter->pen();
    painter->setPen(Qt::gray);
    painter->drawLine(option.rect.bottomLeft(), option.rect.bottomRight());
    painter->drawLine(option.rect.topRight(), option.rect.bottomRight());
    painter->setPen(pen);
}

bool DownloadDelegate::editorEvent(QEvent *e, QAbstractItemModel* model, const QModelIndex &index)
{
    bool typeOk = e && (e->type() == QEvent::MouseButtonPress
                     || e->type() == QEvent::MouseButtonDblClick);
    if (!typeOk || index.column() != 0)
        return QItemDelegate::editorEvent(e, model, index);

    if (static_cast<QMouseEvent*>(e)->x() < 20) {
        bool checked = model->data(index, DownloadDelegate::CheckedRole).toBool();
        model->setData(index, DownloadDelegate::CheckedRole, !checked);
        return true;
    }

    return false;
}

QSize DownloadDelegate::sizeHint(const QStyleOptionViewItem &option,
                                 const QAbstractItemModel *model,
                                 const QModelIndex &index) const
{
    if (index.column() == 2)
        return QSize();

    if (index.column() == 3) {
        int rating = model->data(index, QAbstractItemModel::DisplayRole).toInt();
        return QSize(rating * star.width(), star.height());
    }
    
    return QItemDelegate::sizeHint(option, model, index);    
}

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  QTreeWidget *view = new QTreeWidget;
  view->setAlternatingRowColors(true);
  view->setOddRowColor(0x00f3f3f3);
  view->setRootIsDecorated(false);
  view->setItemDelegate(new DownloadDelegate(view));
  view->setSortingEnabled(true);

  QStringList headerLabels;
  headerLabels << "Name" << "Released" << "Download" << "Rating";
  view->setHeaderLabels(headerLabels);
  view->header()->setResizeMode(QHeaderView::Stretch, 3);

  for (int i = 0; i < 10; ++i) {
      QTreeWidgetItem *item = new QTreeWidgetItem(view);
      item->setText(0, "Song " + QString::number(i));
      item->setData(0, DownloadDelegate::CheckedRole, (i % 5) != 0);
      item->setData(1, DownloadDelegate::DateRole, QDate(2004, 9, 11 + i));
      item->setData(2, DownloadDelegate::ProgressRole, 0.25 * (i % 4) + 0.10);
      item->setData(3, DownloadDelegate::RatingRole, (i % 6) + 1);
      item->setFlags(item->flags()|QAbstractItemModel::ItemIsEditable);
  }

  app.setMainWidget(view);
  view->show();
  return app.exec();
}

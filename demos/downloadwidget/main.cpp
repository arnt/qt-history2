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
#include <qevent.h>

class DownloadDelegate : public QItemDelegate
{
public:
    DownloadDelegate(QObject *parent);
    ~DownloadDelegate();

    enum Roles {
        CheckRole = QAbstractItemModel::CheckStateRole,
        DateRole = QAbstractItemModel::DisplayRole,
        ProgressRole = QAbstractItemModel::DisplayRole,
        RatingRole = QAbstractItemModel::DisplayRole
    };  
    
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
    QPixmap star;
};

DownloadDelegate::DownloadDelegate(QObject *parent)
    : QItemDelegate(parent), star(QPixmap(":images/star.png"))
{
}

DownloadDelegate::~DownloadDelegate()
{
}

void DownloadDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
{
    if (index.column() < 2 || index.column() > 3) {
        QItemDelegate::paint(painter, option, index);
    } else {
        const QAbstractItemModel *model = index.model();
        QPalette::ColorGroup cg = option.state & QStyle::State_Enabled
                                  ? QPalette::Normal : QPalette::Disabled;
        if (option.state & QStyle::State_Selected)
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

    QRect rect(option.rect.x() + 1, option.rect.y() + 1,
               option.rect.width() - 3, option.rect.height() - 3);
    drawFocus(painter, option, rect);

    QPen pen = painter->pen();
    painter->setPen(option.palette.color(QPalette::Mid));
    painter->drawLine(option.rect.bottomLeft(), option.rect.bottomRight());
    painter->drawLine(option.rect.topRight(), option.rect.bottomRight());
    painter->setPen(pen);
}

QSize DownloadDelegate::sizeHint(const QStyleOptionViewItem &option,
                                 const QModelIndex &index) const
{
    if (!index.isValid() || index.column() == 2)
        return QSize();

    if (index.column() == 3) {
        int rating = index.model()->data(index, QAbstractItemModel::DisplayRole).toInt();
        return QSize(rating * star.width(), star.height());
    }
    
    return QItemDelegate::sizeHint(option, index);    
}

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  QTreeWidget *view = new QTreeWidget;
  view->setAlternatingRowColors(true);
  view->setRootIsDecorated(false);
  view->setSortingEnabled(true);
  view->setItemDelegate(new DownloadDelegate(view));

  QStringList headerLabels;
  headerLabels << "Name" << "Released" << "Download" << "Rating";
  view->setHeaderLabels(headerLabels);
  view->header()->setClickable(true);

  for (int i = 0; i < 10; ++i) {
      QTreeWidgetItem *item = new QTreeWidgetItem(view);
      item->setText(0, "Song " + QString::number(i));
      item->setData(0, DownloadDelegate::CheckRole, (i % 5) != 0);
      item->setData(1, DownloadDelegate::DateRole, QDate(2004, 9, 11 + i));
      item->setData(2, DownloadDelegate::ProgressRole, 0.25 * (i % 4) + 0.10);
      item->setData(3, DownloadDelegate::RatingRole, (i % 6) + 1);
      item->setFlags(item->flags()
                     |QAbstractItemModel::ItemIsEditable
                     |QAbstractItemModel::ItemIsCheckable);
  }

  view->setWindowIcon(QPixmap(":/images/interview.png"));
  app.setMainWidget(view);
  view->resize(500, 150);
  view->show();
  return app.exec();
}

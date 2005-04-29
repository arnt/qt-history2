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
#include <qdebug.h>

class DownloadDelegate : public QItemDelegate
{
public:
    DownloadDelegate(QObject *parent);
    ~DownloadDelegate();

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const;

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
        QStyleOptionViewItem opt = option;
        opt.rect.adjust(0, 0, -1, -1); // since we draw the grid ourselves
        QItemDelegate::paint(painter, opt, index);
    } else {
        const QAbstractItemModel *model = index.model();
        QPalette::ColorGroup cg = option.state & QStyle::State_Enabled
                                  ? QPalette::Normal : QPalette::Disabled;
        if (option.state & QStyle::State_Selected)
            painter->fillRect(option.rect, option.palette.color(cg, QPalette::Highlight));
        if (index.column() == 2) {
            QRect rect = option.rect.adjusted(3, 3, -4, -4);
            int download = model->data(index, Qt::DisplayRole).toInt();
            int width = qMin(rect.width(), static_cast<int>(rect.width() * download / 100));
            QLinearGradient gradient(rect.topLeft(), rect.bottomLeft());
            gradient.setColorAt(0, option.palette.color(cg, QPalette::Light));
            gradient.setColorAt(1, option.palette.color(cg, QPalette::Dark));
            painter->fillRect(rect.x(), rect.y(), width, rect.height(), gradient);
            painter->fillRect(rect.x() + width, rect.y(), rect.width() - width, rect.height(),
                              option.palette.base());
            painter->drawRect(rect.adjusted(0, 0, -1, -1));
        } else if (index.column() == 3) {
            int rating = model->data(index, Qt::DisplayRole).toInt();
            int width = star.width();
            int height = star.height();
            int x = option.rect.x();
            int y = option.rect.y() + (option.rect.height() / 2) - (height / 2);
            for (int i = 0; i < rating; ++i) {
                painter->drawPixmap(x, y, star);
                x += width;
            }
        }
        drawFocus(painter, option, option.rect.adjusted(0, 0, -1, -1)); // since we draw the grid ourselves
    }

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
        int rating = index.model()->data(index, Qt::DisplayRole).toInt();
        return QSize(rating * star.width(), star.height()) + QSize(1, 1);
    }

    return QItemDelegate::sizeHint(option, index) + QSize(1, 1); // since we draw the grid ourselves
}

void DownloadDelegate::updateEditorGeometry(QWidget *editor,
                                            const QStyleOptionViewItem &option,
                                            const QModelIndex &index) const
{
    QItemDelegate::updateEditorGeometry(editor, option, index);
    editor->setGeometry(editor->geometry().adjusted(0, 0, -1, -1)); // since we draw the grid ourselves
}


int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  QTreeWidget *view = new QTreeWidget;
  view->setAttribute(Qt::WA_DeleteOnClose);
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
      item->setData(0, Qt::CheckStateRole, (i % 5) != 0 ? Qt::Checked : Qt::Unchecked);
      item->setData(1, Qt::DisplayRole, QDate(2004, 9, 11 + i));
      item->setData(2, Qt::DisplayRole, 25 * (i % 4) + 10);
      item->setData(3, Qt::DisplayRole, (i % 6) + 1);
      item->setFlags(item->flags()|Qt::ItemIsEditable);
  }

  view->setWindowIcon(QPixmap(":/images/interview.png"));
  view->resize(500, 150);
  view->show();
  return app.exec();
}

#include <QtGui>

#include "bookdelegate.h"

BookDelegate::BookDelegate(QObject *parent)
    : QSqlRelationalDelegate(parent), star(QPixmap(":images/star.png"))
{
}

void BookDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                           const QModelIndex &index) const
{
    if (index.column() != 5) {
        QStyleOptionViewItem opt = option;
        opt.rect.adjust(0, 0, -1, -1); // since we draw the grid ourselves
        QSqlRelationalDelegate::paint(painter, opt, index);
    } else {
        const QAbstractItemModel *model = index.model();
        QPalette::ColorGroup cg = option.state & QStyle::State_Enabled
                                  ? QPalette::Normal : QPalette::Disabled;
        if (option.state & QStyle::State_Selected)
            painter->fillRect(option.rect, option.palette.color(cg, QPalette::Highlight));

            int rating = model->data(index, Qt::DisplayRole).toInt();
            int width = star.width();
            int height = star.height();
            int x = option.rect.x();
            int y = option.rect.y() + (option.rect.height() / 2) - (height / 2);
            for (int i = 0; i < rating; ++i) {
                painter->drawPixmap(x, y, star);
                x += width;
        }
        drawFocus(painter, option, option.rect.adjusted(0, 0, -1, -1)); // since we draw the grid ourselves
    }

    QPen pen = painter->pen();
    painter->setPen(option.palette.color(QPalette::Mid));
    painter->drawLine(option.rect.bottomLeft(), option.rect.bottomRight());
    painter->drawLine(option.rect.topRight(), option.rect.bottomRight());
    painter->setPen(pen);
}

QSize BookDelegate::sizeHint(const QStyleOptionViewItem &option,
                                 const QModelIndex &index) const
{
    if (index.column() == 5) {
        int rating = index.model()->data(index, Qt::DisplayRole).toInt();
        qDebug() << index.column() << rating;
        return QSize(rating * star.width(), star.height()) + QSize(1, 1);
    }

    return QSqlRelationalDelegate::sizeHint(option, index) + QSize(1, 1); // since we draw the grid ourselves
}

void BookDelegate::updateEditorGeometry(QWidget *editor,
                                            const QStyleOptionViewItem &option,
                                            const QModelIndex &index) const
{
    QSqlRelationalDelegate::updateEditorGeometry(editor, option, index);
    editor->setGeometry(editor->geometry().adjusted(0, 0, -1, -1)); // since we draw the grid ourselves
}

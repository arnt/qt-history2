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
        return QSize(rating * star.width(), star.height()) + QSize(1, 1);
    }

    return QSqlRelationalDelegate::sizeHint(option, index) + QSize(1, 1); // since we draw the grid ourselves
}

QWidget *BookDelegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.column() == 5) {
        QSpinBox *editor = new QSpinBox(parent);
        editor->setMinimum(0);
        editor->setMaximum(5);
        editor->installEventFilter(const_cast<BookDelegate*>(this));

        return editor;
    } else
        return QSqlRelationalDelegate::createEditor(parent, option, index);
}

void BookDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (index.column() == 5) {
        int value = index.model()->data(index, Qt::DisplayRole).toInt();

        QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
        spinBox->setValue(value);
    } else
        QSqlRelationalDelegate::setEditorData(editor, index);
}

void BookDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                const QModelIndex &index) const
{
    if (index.column() == 5) {
        QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
        spinBox->interpretText();
        int value = spinBox->value();

        model->setData(index, value);
    } else
        QSqlRelationalDelegate::setModelData(editor, model, index);
}

void BookDelegate::updateEditorGeometry(QWidget *editor,
                                            const QStyleOptionViewItem &option,
                                            const QModelIndex &index) const
{
    QSqlRelationalDelegate::updateEditorGeometry(editor, option, index);
    editor->setGeometry(editor->geometry().adjusted(0, 0, -1, -1)); // since we draw the grid ourselves
}

#include "pimdelegate.h"
#include "pimmodel.h"
#include <qpainter.h>

PimDelegate::PimDelegate(QObject *parent)
    : QAbstractItemDelegate(parent)
{

}

PimDelegate::~PimDelegate()
{

}

void PimDelegate::paint(QPainter *painter,
                        const QStyleOptionViewItem &option,
                        const QModelIndex &index) const
{

    const QAbstractItemModel *model = index.model();
    Q_ASSERT(model);

    bool selected = option.state & QStyle::Style_Selected;
    painter->fillRect(option.rect, selected ? Qt::lightGray : Qt::white);

    QPixmap photo = model->data(index, PimModel::PhotoRole).toPixmap();
    QString firstName = model->data(index, PimModel::FirstNameRole).toString();
    QString lastName = model->data(index, PimModel::LastNameRole).toString();
    QString jobTitle = model->data(index, PimModel::JobTitleRole).toString();

    QSize photoSize(64, 64);
    painter->drawPixmap(QRect(option.rect.topLeft(), photoSize), photo);
    int h = painter->fontMetrics().height();
    int x = photoSize.width() + 8;
    int y = option.rect.top() + h;
    painter->drawText(x, y, firstName);
    y += h;
    painter->drawText(x, y, lastName);
    y += h;
    painter->drawText(x, y, jobTitle);
}

QSize PimDelegate::sizeHint(const QStyleOptionViewItem &option,
                            const QModelIndex &index) const
{
    const QAbstractItemModel *model = index.model();
    Q_ASSERT(model);

    QPixmap photo = model->data(index, PimModel::PhotoRole).toPixmap();
    QString firstName = model->data(index, PimModel::FirstNameRole).toString();
    QString lastName = model->data(index, PimModel::LastNameRole).toString();
    QString jobTitle = model->data(index, PimModel::JobTitleRole).toString();

    QRect rect = photo.rect();
    int h = qMax(option.fontMetrics.height() * 3, rect.height());
    int w = option.fontMetrics.width(firstName);
    w = qMax(option.fontMetrics.width(lastName), w);
    w = qMax(option.fontMetrics.width(jobTitle), w);
    w += rect.width() + 8;

    return QSize(w, h);
}

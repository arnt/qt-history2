#include "pimdelegate.h"
#include "pimmodel.h"
#include <qpainter.h>
#include <qdebug.h>

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
    const PimModel *model = qobject_cast<const PimModel*>(index.model());
    Q_ASSERT(model); // if the model is not a PimModel we could have a fallback here
    const PimEntry &entry = model->entry(index);

    bool selected = option.state & QStyle::State_Selected;
    painter->fillRect(option.rect, selected ? Qt::lightGray : Qt::white);

    QSize photoSize(64, 64);
    QPoint topLeft = option.rect.topLeft();
    painter->drawPixmap(QRect(topLeft, photoSize), entry.photo);
    int h = painter->fontMetrics().height();
    int x = topLeft.x() + photoSize.width() + 8;
    int y = topLeft.y() + h;
    painter->drawText(x, y, entry.firstName);
    y += h;
    painter->drawText(x, y, entry.lastName);
    y += h;
    painter->drawText(x, y, entry.jobTitle);
}

QSize PimDelegate::sizeHint(const QStyleOptionViewItem &option,
                            const QModelIndex &index) const
{
    const PimModel *model = qobject_cast<const PimModel*>(index.model());
    Q_ASSERT(model); // if the model is not a PimModel we could have a fallback here
    const PimEntry &entry = model->entry(index);

    QRect rect = entry.photo.rect();
    int h = qMax(option.fontMetrics.height() * 3, rect.height());
    int w = option.fontMetrics.width(entry.firstName);
    w = qMax(option.fontMetrics.width(entry.lastName), w);
    w = qMax(option.fontMetrics.width(entry.jobTitle), w);
    w += rect.width() + 8;

    return QSize(w, h);
}

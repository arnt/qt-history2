#include "qtoolbarseparator_p.h"

#include <qstyle.h>
#include <qstyleoption.h>
#include <qtoolbar.h>

QToolBarSeparator::QToolBarSeparator(Qt::Orientation orientation, QToolBar *parent)
    : QWidget(parent), orient(orientation)
{
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
}

QSize QToolBarSeparator::sizeHint() const
{
    const int extent = style().pixelMetric(QStyle::PM_DockWindowSeparatorExtent, this);
    return QSize(extent, extent);
}

void QToolBarSeparator::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    QStyleOption opt(0, QStyleOption::SO_Default);
    opt.rect = rect();
    opt.palette = palette();
    if (orientation() == Qt::Horizontal)
        opt.state = QStyle::Style_Horizontal;
    else
        opt.state = QStyle::Style_Default;

    style().drawPrimitive(QStyle::PE_DockWindowSeparator, &opt, &p, this);
}

#include "pimviewer.h"
#include "pimmodel.h"
#include <qabstractitemview.h>
#include <qpainter.h>
#include <qevent.h>

PimViewer::PimViewer(QWidget *parent)
    : QWidget(parent), m(0), entry(-1)
{

}

PimViewer::~PimViewer()
{

}

void PimViewer::setModel(const PimModel *model)
{
    m = model;
}

const PimModel *PimViewer::model() const
{
    return m;
}

void PimViewer::view(const QModelIndex &index)
{
    entry = index.row();
}

void PimViewer::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    p.fillRect(event->rect(), Qt::white);
    if (m && entry > -1) {
        const PimEntry &pe = m->entry(entry);
        QRect r = pe.photo.rect();
        r.moveCenter(rect().center());
        int y = 8;
        p.drawPixmap(QRect(r.x(), y, r.width(), r.height()), pe.photo);
        y += pe.photo.height();
        int h = p.fontMetrics().height();
        p.drawText(QRect(0, y, width(), h), Qt::AlignCenter, pe.firstName);
        y += h;
        p.drawText(QRect(0, y, width(), h), Qt::AlignCenter, pe.lastName);
        y += h;
        p.drawText(QRect(0, y, width(), h), Qt::AlignCenter, pe.middleName);
        y += h;
        p.drawText(QRect(0, y, width(), h), Qt::AlignCenter, pe.company);
        y += h;
        p.drawText(QRect(0, y, width(), h), Qt::AlignCenter, pe.department);
        y += h;
        p.drawText(QRect(0, y, width(), h), Qt::AlignCenter, pe.jobTitle);
    }
}

void PimViewer::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    emit done();
}

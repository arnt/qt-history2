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

#include "spacer_widget.h"
#include "layoutinfo.h"

#include <abstractformwindow.h>

#include <QLayout>
#include <QPainter>
#include <QtCore/qdebug.h>

Spacer::Spacer(QWidget *parent)
    : QWidget(parent),
      orient(Qt::Vertical), interactive(true), sh(20, 40)
{
    setAttribute(Qt::WA_MouseNoMask);
    m_formWindow = AbstractFormWindow::findFormWindow(this);

    setSizeType(Spacer::Expanding);
    setAutoMask(true);
}

void Spacer::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setPen(Qt::blue);

    if (orient == Qt::Horizontal) {
        const int dist = 3;
        const int amplitude = qMin(3, height() / 3);
        const int base = height() / 2;
        int i = 0;
        p.setPen(Qt::white);
        for (i = 0; i < width() / 3 +2; ++i)
            p.drawLine(i * dist, base - amplitude, i * dist + dist / 2, base + amplitude);
        p.setPen(Qt::blue);
        for (i = 0; i < width() / 3 +2; ++i)
            p.drawLine(i * dist + dist / 2, base + amplitude, i * dist + dist, base - amplitude);
        int y = height()/2;
        p.drawLine(0, y-10, 0, y+10);
        p.drawLine(width() - 1, y-10, width() - 1, y+10);
    } else {
        const int dist = 3;
        const int amplitude = qMin(3, width() / 3);
        const int base = width() / 2;
        int i = 0;
        p.setPen(Qt::white);
        for (i = 0; i < height() / 3 +2; ++i)
            p.drawLine(base - amplitude, i * dist, base + amplitude,i * dist + dist / 2);
        p.setPen(Qt::blue);
        for (i = 0; i < height() / 3 +2; ++i)
            p.drawLine(base + amplitude, i * dist + dist / 2, base - amplitude, i * dist + dist);
        int x = width()/2;
        p.drawLine(x-10, 0, x+10, 0);
        p.drawLine(x-10, height() - 1, x+10, height() - 1);
    }
}

void Spacer::resizeEvent(QResizeEvent* e)
{
    QWidget::resizeEvent(e);

    if (!interactive)
        return;

    if (!parentWidget() || (m_formWindow && LayoutInfo::layoutType(m_formWindow->core(), parentWidget()) == LayoutInfo::NoLayout))
        sh = size();
}

void Spacer::updateMask()
{
    QRegion r(rect());
    if (orient == Qt::Horizontal) {
        const int amplitude = qMin(3, height() / 3);
        const int base = height() / 2;
        r = r.subtract(QRect(1, 0, width() - 2, base - amplitude));
        r = r.subtract(QRect(1, base + amplitude, width() - 2, height() - base - amplitude));
    } else {
        const int amplitude = qMin(3, width() / 3);
        const int base = width() / 2;
        r = r.subtract(QRect(0, 1, base - amplitude, height() - 2));
        r = r.subtract(QRect(base + amplitude, 1, width() - base - amplitude, height() - 2));
    }
    setMask(r);
}

void Spacer::setSizeType(Spacer::SizeType t)
{
    QSizePolicy sizeP;
    if (orient == Qt::Vertical)
        sizeP = QSizePolicy(QSizePolicy::Minimum, (QSizePolicy::SizeType)t);
    else
        sizeP = QSizePolicy((QSizePolicy::SizeType)t, QSizePolicy::Minimum);
    setSizePolicy(sizeP);
}


Spacer::SizeType Spacer::sizeType() const
{
    if (orient == Qt::Vertical)
        return (Spacer::SizeType) int(sizePolicy().verticalData());
    return (Spacer::SizeType) int(sizePolicy().horizontalData());
}

Qt::Alignment Spacer::alignment() const
{
    if (orient == Qt::Vertical)
        return Qt::AlignHCenter;
    return Qt::AlignVCenter;
}

QSize Spacer::minimumSize() const
{
    QSize s = QSize(20,20);
    if (sizeType() == Spacer::Expanding)
        if (orient == Qt::Vertical)
            s.rheight() = 0;
        else
            s.rwidth() = 0;
    return s;
}

QSize Spacer::sizeHint() const
{
    return sh;
}

void Spacer::setSizeHint(const QSize &s)
{
    sh = s;

    if (!parentWidget() || (m_formWindow && LayoutInfo::layoutType(m_formWindow->core(), parentWidget()) == LayoutInfo::NoLayout))
        resize(sizeHint());

    updateGeometry();
}

Qt::Orientation Spacer::orientation() const
{
    return orient;
}

void Spacer::setOrientation(Qt::Orientation o)
{
    if (orient == o)
        return;

    Spacer::SizeType st = sizeType();
    orient = o;
    setSizeType(st);

    if (interactive) {
        sh = QSize(sh.height(), sh.width());
        if (!parentWidget() || (m_formWindow && LayoutInfo::layoutType(m_formWindow->core(), parentWidget()) == LayoutInfo::NoLayout))
            resize(height(), width());
    }

    updateMask();
    update();
    updateGeometry();
}

/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "spacer_widget_p.h"
#include "layoutinfo_p.h"

#include <QtDesigner/abstractformwindow.h>
#include <QtDesigner/QDesignerFormWindowInterface>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerPropertySheetExtension>
#include <QtDesigner/QExtensionManager>

#include <QtGui/QLayout>
#include <QtGui/QPainter>
#include <QtGui/qevent.h>
#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

using namespace qdesigner_internal;

Spacer::Spacer(QWidget *parent) :
    QWidget(parent),
    m_orientation(Qt::Vertical),
    m_interactive(true),
    m_sizeHint(0, 0)
{
    setAttribute(Qt::WA_MouseNoMask);
    m_formWindow = QDesignerFormWindowInterface::findFormWindow(this);
    setSizeType(QSizePolicy::Expanding);
}

void Spacer::paintEvent(QPaintEvent *)
{
    // Only draw spacers when we're editting widgets
    if (m_formWindow != 0 && m_formWindow->currentTool() != 0)
        return;

    QPainter p(this);
    p.setPen(Qt::blue);
    const int w = width();
    const int h = height();
    if (w < 2 && h < 2)
        return;
    if (m_orientation == Qt::Horizontal) {
        const int dist = 3;
        const int amplitude = qMin(3, h / 3);
        const int base = h / 2;
        int i = 0;
        p.setPen(Qt::white);
        for (i = 0; i < w / 3 +2; ++i)
            p.drawLine(i * dist, base - amplitude, i * dist + dist / 2, base + amplitude);
        p.setPen(Qt::blue);
        for (i = 0; i < w / 3 +2; ++i)
            p.drawLine(i * dist + dist / 2, base + amplitude, i * dist + dist, base - amplitude);
        const int y = h/2;
        p.drawLine(0, y-10, 0, y+10);
        p.drawLine(w - 1, y-10, w - 1, y+10);
    } else {
        const int dist = 3;
        const int amplitude = qMin(3, w / 3);
        const int base = w / 2;
        int i = 0;
        p.setPen(Qt::white);
        for (i = 0; i < h / 3 +2; ++i)
            p.drawLine(base - amplitude, i * dist, base + amplitude,i * dist + dist / 2);
        p.setPen(Qt::blue);
        for (i = 0; i < h / 3 +2; ++i)
            p.drawLine(base + amplitude, i * dist + dist / 2, base - amplitude, i * dist + dist);
        const int x = w/2;
        p.drawLine(x-10, 0, x+10, 0);
        p.drawLine(x-10, h - 1, x+10, h - 1);
    }
}

void Spacer::resizeEvent(QResizeEvent* e)
{
    QWidget::resizeEvent(e);
    // When resized by widget handle dragging after a reset (QSize(0, 0)):
    // Mark the property as changed (geometry and sizeHint are in sync except for 'changed').
    if (e->oldSize().isNull() && m_formWindow)
        if (QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(m_formWindow->core()->extensionManager(), this))
            sheet->setChanged(sheet->indexOf(QLatin1String("sizeHint")), true);

    updateMask();

    if (!m_interactive)
        return;

    if (!parentWidget() || (m_formWindow && LayoutInfo::layoutType(m_formWindow->core(), parentWidget()) == LayoutInfo::NoLayout))
        m_sizeHint = size();
}

void Spacer::updateMask()
{
    QRegion r(rect());
    const int w = width();
    const int h = height();
    if (w > 1 && h > 1) {
        if (m_orientation == Qt::Horizontal) {
            const int amplitude = qMin(3, h / 3);
            const int base = h / 2;
            r = r.subtract(QRect(1, 0, w - 2, base - amplitude));
            r = r.subtract(QRect(1, base + amplitude, w - 2, h - base - amplitude));
        } else {
            const int amplitude = qMin(3, w / 3);
            const int base = w / 2;
            r = r.subtract(QRect(0, 1, base - amplitude, h - 2));
            r = r.subtract(QRect(base + amplitude, 1, w - base - amplitude, h - 2));
        }
    }
    setMask(r);
}

void Spacer::setSizeType(QSizePolicy::Policy t)
{
    const QSizePolicy sizeP = m_orientation == Qt::Vertical ? QSizePolicy(QSizePolicy::Minimum, t) : QSizePolicy(t, QSizePolicy::Minimum);
    setSizePolicy(sizeP);
}


QSizePolicy::Policy Spacer::sizeType() const
{
    return m_orientation == Qt::Vertical ? sizePolicy().verticalPolicy() : sizePolicy().horizontalPolicy();
}

Qt::Alignment Spacer::alignment() const
{
    // For grid layouts
    return m_orientation == Qt::Vertical ?  Qt::AlignHCenter : Qt::AlignVCenter;
}

QSize Spacer::sizeHint() const
{
    return m_sizeHint;
}

void Spacer::setSizeHint(const QSize &s)
{
    m_sizeHint = s;

    if (!parentWidget() || (m_formWindow && LayoutInfo::layoutType(m_formWindow->core(), parentWidget()) == LayoutInfo::NoLayout))
        resize(s);

    updateGeometry();
}

Qt::Orientation Spacer::orientation() const
{
    return m_orientation;
}

void Spacer::setOrientation(Qt::Orientation o)
{
    if (m_orientation == o)
        return;

    const QSizePolicy::Policy st = sizeType(); // flip size type
    m_orientation = o;
    setSizeType(st);

    if (m_interactive) {
        m_sizeHint = QSize(m_sizeHint.height(), m_sizeHint.width());
        if (!parentWidget() || (m_formWindow && LayoutInfo::layoutType(m_formWindow->core(), parentWidget()) == LayoutInfo::NoLayout))
            resize(height(), width());
    }

    updateMask();
    update();
    updateGeometry();
}

QT_END_NAMESPACE

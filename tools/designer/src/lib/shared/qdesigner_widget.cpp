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

#include "qdesigner_widget_p.h"
#include "qdesigner_command_p.h"
#include "layout_p.h"
#include "invisible_widget_p.h"

#include <QtDesigner/QtDesigner>
#include <QtDesigner/QExtensionManager>

#include <QtGui/QBitmap>
#include <QtGui/QToolButton>
#include <QtGui/QPainter>
#include <QtGui/QApplication>
#include <QtGui/QLayout>
#include <QtGui/QAction>
#include <QtGui/QMessageBox>
#include <QtGui/qevent.h>

#include <QtCore/qdebug.h>

using namespace qdesigner_internal;

static void paintGrid(QWidget *widget, QDesignerFormWindowInterface *formWindow, QPaintEvent *e, bool needFrame = false)
{
    QPainter p(widget);

    p.fillRect(e->rect(), widget->palette().brush(widget->backgroundRole()));

    p.setPen(widget->palette().dark().color());
    int width = widget->width();
    int height = widget->height();
    int pointCount = qRound((width/float(formWindow->grid().x()))+.5) * qRound((height/float(formWindow->grid().y()))+.5);
    static const int BUF_SIZE = 4096;
    QPoint points[BUF_SIZE];

    int x = 0;
    int y = 0;
    int i = 0;
    while (pointCount > 0) {
        while (i < pointCount && i < BUF_SIZE) {
            points[i] = QPoint(x, y);
            ++i;
            x += formWindow->grid().x();
            if (x >= width) {
                x = 0;
                y += formWindow->grid().y();
                if (y > height) // probably never reached..
                    break;
            }
        }
        p.drawPoints(points, i);
        pointCount -= i;
        i = 0;
    }
    if (needFrame) {
        p.setPen(widget->palette().dark().color());
        p.drawRect(widget->rect());
    }
}

void QDesignerDialog::paintEvent(QPaintEvent *e)
{
    if (m_formWindow->currentTool() == 0 && m_formWindow->hasFeature(QDesignerFormWindowInterface::GridFeature)) {
        paintGrid(this, m_formWindow, e);
    } else {
        QPainter p(this);
        p.fillRect(rect(), palette().brush(QPalette::Background));
    }
}

void QDesignerLabel::updateBuddy()
{
    if (myBuddy.isEmpty())
        return;

    if (QWidget *widget = qFindChild<QWidget*>(topLevelWidget(), QString::fromUtf8(myBuddy)))
        QLabel::setBuddy(widget);
}

QDesignerWidget::QDesignerWidget(QDesignerFormWindowInterface* formWindow, QWidget *parent)
    : QWidget(parent), m_formWindow(formWindow)
{
    need_frame = true;
    setBackgroundRole(QPalette::Background);
}

QDesignerWidget::~QDesignerWidget()
{
}

void QDesignerWidget::paintEvent(QPaintEvent *e)
{
    if (m_formWindow->currentTool() == 0 && m_formWindow->hasFeature(QDesignerFormWindowInterface::GridFeature))
        paintGrid(this, m_formWindow, e);
    else
        QWidget::paintEvent(e);
}

void QDesignerWidget::dragEnterEvent(QDragEnterEvent *)
{
//    e->setAccepted(QTextDrag::canDecode(e));
}

QDesignerLabel::QDesignerLabel(QWidget *parent)
    : QLabel(parent)
{
}

void QDesignerLabel::setBuddy(QWidget *widget)
{
    QLabel::setBuddy(widget);
}

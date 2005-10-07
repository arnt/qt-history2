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
#include <QtGui/QPixmapCache>
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
    p.setClipRect(e->rect());

    p.fillRect(e->rect(), widget->palette().brush(widget->backgroundRole()));

    QString grid_name;
    grid_name.sprintf("__qt_designer_grid_%d_%d", formWindow->grid().x(), formWindow->grid().y());

    QPixmap grid;
    if (!QPixmapCache::find(grid_name, grid)) {
        grid = QPixmap(350 + (350 % formWindow->grid().x()), 350 + (350 % formWindow->grid().y()));
        grid.fill(widget->palette().foreground().color());
        QBitmap mask(grid.width(), grid.height());
        mask.fill(Qt::color0);
        QPainter p(&mask);
        p.setPen(Qt::color1);
        for (int y = 0; y < grid.width(); y += formWindow->grid().y()) {
            for (int x = 0; x < grid.height(); x += formWindow->grid().x()) {
                p.drawPoint(x, y);
            }
        }
        p.end();
        grid.setMask(mask);
        QImage image = grid.toImage(); // ### workaround
        grid = QPixmap::fromImage(image);
        QPixmapCache::insert(grid_name, grid);
    }

    p.drawTiledPixmap(0, 0, widget->width(), widget->height(), grid);

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
        QDialog::paintEvent(e);
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
}

QDesignerWidget::~QDesignerWidget()
{
}

void QDesignerWidget::paintEvent(QPaintEvent *e)
{
    if (m_formWindow->currentTool() == 0 && m_formWindow->hasFeature(QDesignerFormWindowInterface::GridFeature)) {
        paintGrid(this, m_formWindow, e);
    } else {
        QWidget::paintEvent(e);
    }
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

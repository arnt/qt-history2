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

#include "sizehandle.h"
#include "formwindow.h"
#include "formwindowmanager.h"
#include "layoutinfo.h"
#include "command.h"

#include <abstractwidgetfactory.h>

#include <QVariant>
#include <QWidget>
#include <QApplication>
#include <QLabel>
#include <QPainter>
#include <QMouseEvent>

#include <qdebug.h>

SizeHandle::SizeHandle(FormWindow *parent, Direction d, WidgetSelection *s)
    : QWidget(parent)
{
    active = true;
    setBackgroundRole(active ? QPalette::Text : QPalette::Dark);
    setFixedSize(6, 6);
    widget = 0;
    dir =d ;
    setMouseTracking(false);
    formWindow = parent;
    sel = s;
    updateCursor();
}

void SizeHandle::updateCursor()
{
    if (!active) {
        setCursor(Qt::ArrowCursor);
        return;
    }

    switch (dir) {
    case LeftTop:
        setCursor(Qt::SizeFDiagCursor);
        break;
    case Top:
        setCursor(Qt::SizeVerCursor);
        break;
    case RightTop:
        setCursor(Qt::SizeBDiagCursor);
        break;
    case Right:
        setCursor(Qt::SizeHorCursor);
        break;
    case RightBottom:
        setCursor(Qt::SizeFDiagCursor);
        break;
    case Bottom:
        setCursor(Qt::SizeVerCursor);
        break;
    case LeftBottom:
        setCursor(Qt::SizeBDiagCursor);
        break;
    case Left:
        setCursor(Qt::SizeHorCursor);
        break;
    }
}

void SizeHandle::setActive(bool a)
{
    active = a;
    setBackgroundRole(active ? QPalette::Text : QPalette::Dark);
    updateCursor();
}

void SizeHandle::setWidget(QWidget *w)
{
    widget = w;
}

void SizeHandle::paintEvent(QPaintEvent *)
{
    FormWindow *fw = static_cast<FormWindow*>(parentWidget());
    if (fw->currentWidget() != widget)
        return;

    AbstractFormWindowManager *m = fw->core()->formWindowManager();

    QPainter p(this);
    p.setPen(m->activeFormWindow() == fw ? Qt::blue : Qt::red);
    p.drawRect(0, 0, width(), height());
}

void SizeHandle::mousePressEvent(QMouseEvent *e)
{
    e->accept();

    if (!widget || e->button() != Qt::LeftButton || !active)
        return;

    if (!formWindow->hasFeature(FormWindow::EditFeature))
        return;
    
    QWidget *container = widget->parentWidget();

    oldPressPos = container->mapFromGlobal(e->globalPos());
    geom = origGeom = widget->geometry();
}

int SizeHandle::adjustPoint(int x, int dx)
{ return (x / dx) * dx + 1; }

void SizeHandle::mouseMoveEvent(QMouseEvent *e)
{
    if (!(widget && active && e->buttons() & Qt::LeftButton))
        return;

    //e->accept();

    QWidget *container = widget->parentWidget();

    QPoint rp = container->mapFromGlobal(e->globalPos());
    QPoint d = rp - oldPressPos;
    oldPressPos = rp;

    QRect pr = container->rect();
    QPoint grid = formWindow->grid();

    switch (dir) {

    case LeftTop: {
        if (rp.x() > pr.width() - 2 * width() || rp.y() > pr.height() - 2 * height())
            return;

        int w = geom.width() - d.x();
        geom.setWidth(w);
        w = adjustPoint(w, grid.x());

        int h = geom.height() - d.y();
        geom.setHeight(h);
        h = adjustPoint(h, grid.y());

        int dx = widget->width() - w;
        int dy = widget->height() - h;

        trySetGeometry(widget, widget->x() + dx, widget->y() + dy, w, h);
    } break;

    case Top: {
        if (rp.y() > pr.height() - 2 * height())
            return;

        int h = geom.height() - d.y();
        geom.setHeight(h);
        h = adjustPoint(h, grid.y());

        int dy = widget->height() - h;
        trySetGeometry(widget, widget->x(), widget->y() + dy, widget->width(), h);
    } break;

    case RightTop: {
        if (rp.x() < 2 * width() || rp.y() > pr.height() - 2 * height())
            return;

        int h = geom.height() - d.y();
        geom.setHeight(h);
        h = adjustPoint(h, grid.y());

        int dy = widget->height() - h;

        int w = geom.width() + d.x();
        geom.setWidth(w);
        w = adjustPoint(w, grid.x());

        trySetGeometry(widget, widget->x(), widget->y() + dy, w, h);
    } break;

    case Right: {
        if (rp.x() < 2 * width())
            return;

        int w = geom.width() + d.x();
        geom.setWidth(w);
        w = adjustPoint(w, grid.x());

        tryResize(widget, w, widget->height());
    } break;

    case RightBottom: {
        if (rp.x() < 2 * width() || rp.y() < 2 * height())
            return;

        int w = geom.width() + d.x();
        geom.setWidth(w);
        w = adjustPoint(w, grid.x());

        int h = geom.height() + d.y();
        geom.setHeight(h);
        h = adjustPoint(h, grid.y());

        tryResize(widget, w, h);
    } break;

    case Bottom: {
        if (rp.y() < 2 * height())
            return;

        int h = geom.height() + d.y();
        geom.setHeight(h);
        h = adjustPoint(h, grid.y());

        tryResize(widget, widget->width(), h);
    } break;

    case LeftBottom: {
        if (rp.x() > pr.width() - 2 * width() || rp.y() < 2 * height())
            return;

        int w = geom.width() - d.x();
        geom.setWidth(w);
        w = adjustPoint(w, grid.x());

        int h = geom.height() + d.y();
        geom.setHeight(h);
        h = adjustPoint(h, grid.y());

        int dx = widget->width() - w;

        trySetGeometry(widget, widget->x() + dx, widget->y(), w, h);
    } break;

    case Left: {
        if (rp.x() > pr.width() - 2 * width())
            return;

        int w = geom.width() - d.x();
        geom.setWidth(w);
        w = adjustPoint(w, grid.x());

        int dx = widget->width() - w;

        trySetGeometry(widget, widget->x() + dx, widget->y(), w, widget->height());
    } break;

    default: {
    } break;

    } // end switch

    sel->updateGeometry();

#if 0 // ### enable me
    QPoint p = pos();
    sel->updateGeometry();
    oldPressPos += (p - pos());

    QLabel *sizePreview = formWindow->sizePreview();
    sizePreview->setText(tr("%1/%2").arg(widget->width()).arg(widget->height()));
    sizePreview->adjustSize();
    QRect lg(formWindow->mapFromGlobal(e->globalPos()) + QPoint(16, 16), sizePreview->size());
    formWindow->checkPreviewGeometry(lg);
    sizePreview->setGeometry(lg);
    sizePreview->show();
    sizePreview->raise();
#endif

    if (LayoutInfo::layoutType(formWindow->core(), widget) != LayoutInfo::NoLayout)
        formWindow->updateChildSelections(widget);
}

void SizeHandle::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton || !active)
        return;
    e->accept();

    formWindow->sizePreview()->hide();

    if (!formWindow->hasFeature(FormWindow::EditFeature))
        return;
        
    if (geom != widget->geometry()) {
        SetPropertyCommand *cmd = new SetPropertyCommand(formWindow);
        cmd->init(widget, "geometry", widget->geometry());
        cmd->setOldValue(origGeom);
        formWindow->commandHistory()->push(cmd);
        formWindow->emitSelectionChanged();
    }
}

void SizeHandle::trySetGeometry(QWidget *w, int x, int y, int width, int height)
{
    if (!formWindow->hasFeature(FormWindow::EditFeature))
        return;

    int minw = qMax(w->minimumSizeHint().width(), w->minimumSize().width());
    minw = qMax(minw, 2 * formWindow->grid().x());

    int minh = qMax(w->minimumSizeHint().height(), w->minimumSize().height());
    minh = qMax(minh, 2 * formWindow->grid().y());

    if (qMax(minw, width) > w->maximumWidth() ||
         qMax(minh, height) > w->maximumHeight())
        return;

    if (width < minw && x != w->x())
        x -= minw - width;

    if (height < minh && y != w->y())
        y -= minh - height;

    w->setGeometry(x, y, qMax(minw, width), qMax(minh, height));
}

void SizeHandle::tryResize(QWidget *w, int width, int height)
{
    int minw = qMax(w->minimumSizeHint().width(), w->minimumSize().width());
    minw = qMax(minw, 16);

    int minh = qMax(w->minimumSizeHint().height(), w->minimumSize().height());
    minh = qMax(minh, 16);

    w->resize(qMax(minw, width), qMax(minh, height));
}

// ------------------------------------------------------------------------

WidgetSelection::WidgetSelection(FormWindow *parent, QHash<QWidget *, WidgetSelection *> *selDict)
    : selectionDict(selDict)
{
    formWindow = parent;
    for (int i = SizeHandle::LeftTop; i <= SizeHandle::Left; ++i) {
        handles.insert(i, new SizeHandle(formWindow, (SizeHandle::Direction)i, this));
    }
    hide();
}

void WidgetSelection::setWidget(QWidget *w, bool updateDict)
{
    if (!w) {
        hide();
        if (updateDict)
            selectionDict->remove(wid);
        wid = 0;
        return;
    }

    wid = w;
    bool active = !wid->parentWidget() || LayoutInfo::layoutType(formWindow->core(), wid->parentWidget()) == LayoutInfo::NoLayout;
    for (int i = SizeHandle::LeftTop; i <= SizeHandle::Left; ++i) {
        SizeHandle *h = handles[ i ];
        if (h) {
            h->setWidget(wid);
            h->setActive(active);
        }
    }
    updateGeometry();
    show();
    if (updateDict)
        selectionDict->insert(w, this);
}

bool WidgetSelection::isUsed() const
{
    return wid != 0;
}

void WidgetSelection::updateGeometry()
{
    if (!wid || !wid->parentWidget())
        return;

    QPoint p = wid->parentWidget()->mapToGlobal(wid->pos());
    p = formWindow->mapFromGlobal(p);
    QRect r(p, wid->size());

    int w = 6;
    int h = 6;

    for (int i = SizeHandle::LeftTop; i <= SizeHandle::Left; ++i) {
        SizeHandle *hndl = handles[ i ];
        if (!hndl)
            continue;
        switch (i) {
        case SizeHandle::LeftTop:
            hndl->move(r.x() - w / 2, r.y() - h / 2);
            break;
        case SizeHandle::Top:
            hndl->move(r.x() + r.width() / 2 - w / 2, r.y() - h / 2);
            break;
        case SizeHandle::RightTop:
            hndl->move(r.x() + r.width() - w / 2, r.y() - h / 2);
            break;
        case SizeHandle::Right:
            hndl->move(r.x() + r.width() - w / 2, r.y() + r.height() / 2 - h / 2);
            break;
        case SizeHandle::RightBottom:
            hndl->move(r.x() + r.width() - w / 2, r.y() + r.height() - h / 2);
            break;
        case SizeHandle::Bottom:
            hndl->move(r.x() + r.width() / 2 - w / 2, r.y() + r.height() - h / 2);
            break;
        case SizeHandle::LeftBottom:
            hndl->move(r.x() - w / 2, r.y() + r.height() - h / 2);
            break;
        case SizeHandle::Left:
            hndl->move(r.x() - w / 2, r.y() + r.height() / 2 - h / 2);
            break;
        default:
            break;
        }
    }
}

void WidgetSelection::hide()
{
    for (int i = SizeHandle::LeftTop; i <= SizeHandle::Left; ++i) {
        SizeHandle *h = handles[ i ];
        if (h)
            h->hide();
    }
}

void WidgetSelection::show()
{
    for (int i = SizeHandle::LeftTop; i <= SizeHandle::Left; ++i) {
        SizeHandle *h = handles[ i ];
        if (h) {
            h->show();
            h->raise();
        }
    }
}

void WidgetSelection::update()
{
    for (int i = SizeHandle::LeftTop; i <= SizeHandle::Left; ++i) {
        SizeHandle *h = handles[ i ];
        if (h)
            h->update();
    }
}

QWidget *WidgetSelection::widget() const
{
    return wid;
}

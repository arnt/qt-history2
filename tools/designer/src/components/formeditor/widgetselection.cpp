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

#include "widgetselection.h"
#include "formwindow.h"
#include "formwindowmanager.h"

// sdk
#include <QtDesigner/QtDesigner>
#include <QtDesigner/QExtensionManager>

// shared
#include <qdesigner_command_p.h>
#include <qdesigner_propertycommand_p.h>
#include <layout_p.h>
#include <layoutinfo_p.h>

#include <QtGui/QMenu>
#include <QtGui/QWidget>
#include <QtGui/QMouseEvent>
#include <QtGui/QStylePainter>
#include <QtGui/QGridLayout>
#include <QtGui/QStyleOptionToolButton>

#include <QtCore/QVariant>
#include <QtCore/qdebug.h>

#define NO_TOPWIDGET


namespace qdesigner_internal
{


class TopWidget: public InvisibleWidget
{
    Q_OBJECT
public:
    TopWidget(QWidget *parent)
        : InvisibleWidget(parent)
    {
        setAttribute(Qt::WA_TransparentForMouseEvents);
    }
};

WidgetHandle::WidgetHandle(FormWindow *parent, WidgetHandle::Type t, WidgetSelection *s)
    : InvisibleWidget(parent->mainContainer()),
    m_widget(0),
    m_type(t),
    m_formWindow( parent),
    m_sel(s),
    m_active(true)
{
    setMouseTracking(false);
    setAutoFillBackground(true);

    if (m_type == TaskMenu) {
        setBackgroundRole(QPalette::Button);
        setFixedSize(12, 12);
    } else {
        setBackgroundRole(m_active ? QPalette::Text : QPalette::Dark);
        setFixedSize(6, 6);
    }

    updateCursor();
}

void WidgetHandle::updateCursor()
{
    if (!m_active) {
        setCursor(Qt::ArrowCursor);
        return;
    }

    switch (m_type) {
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
    case TaskMenu:
        setCursor(Qt::ArrowCursor);
        break;
    default:
        Q_ASSERT(0);
    }
}

QDesignerFormEditorInterface *WidgetHandle::core() const
{
    if (m_formWindow)
        return m_formWindow->core();

    return 0;
}

void WidgetHandle::setActive(bool a)
{
    m_active = a;
    if (m_type != TaskMenu) {
        setBackgroundRole(m_active ? QPalette::Text : QPalette::Dark);
    }
    updateCursor();
}

void WidgetHandle::setWidget(QWidget *w)
{
    m_widget = w;
}

void WidgetHandle::paintEvent(QPaintEvent *)
{
    QDesignerFormWindowManagerInterface *m = m_formWindow->core()->formWindowManager();

    QStylePainter p(this);
    if (m_type == TaskMenu) {
        QStyleOptionToolButton option;
        option.init(this);
        option.state |= QStyle::State_Raised;
        option.arrowType = Qt::RightArrow;
        option.toolButtonStyle = Qt::ToolButtonIconOnly;
        option.features = QStyleOptionToolButton::Arrow;
        option.subControls = QStyle::SC_ToolButton;
        p.drawComplexControl(QStyle::CC_ToolButton, option);
    } else if (m_formWindow->currentWidget() == m_widget) {
        p.setPen(m->activeFormWindow() == m_formWindow ? Qt::blue : Qt::red);
        p.drawRect(0, 0, width() - 1, height() - 1);
    }
}

void WidgetHandle::mousePressEvent(QMouseEvent *e)
{
    e->accept();

    if (!m_formWindow->hasFeature(FormWindow::EditFeature))
        return;

    if (!(m_widget && e->button() == Qt::LeftButton))
        return;

    if (!(m_active || m_type == TaskMenu))
        return;

    QWidget *container = m_widget->parentWidget();

    m_origPressPos = container->mapFromGlobal(e->globalPos());
    m_geom = m_origGeom = m_widget->geometry();

    if (m_type == TaskMenu && e->button() == Qt::LeftButton) {
        Q_ASSERT(m_sel->taskMenuExtension());

        QMenu m(this);
        foreach (QAction *a, m_sel->taskMenuExtension()->taskActions()) {
            m.addAction(a);
        }
        m.exec(e->globalPos());
    }

}

int WidgetHandle::adjustPoint(int x, int dx)
{ return (x / dx) * dx + 1; }

void WidgetHandle::mouseMoveEvent(QMouseEvent *e)
{
    if (!(m_widget && m_active && e->buttons() & Qt::LeftButton))
        return;

    if (m_type == TaskMenu)
        return;

    e->accept();

    QWidget *container = m_widget->parentWidget();

    const QPoint rp = container->mapFromGlobal(e->globalPos());
    const QPoint d = rp - m_origPressPos;

    const QRect pr = container->rect();
    const QPoint grid =  m_formWindow->grid();

    switch (m_type) {

    case TaskMenu:
        break;

    case LeftTop: {
        if (rp.x() > pr.width() - 2 * width() || rp.y() > pr.height() - 2 * height())
            return;

        int w = m_origGeom.width() - d.x();
        m_geom.setWidth(w);
        w = adjustPoint(w, grid.x());

        int h = m_origGeom.height() - d.y();
        m_geom.setHeight(h);
        h = adjustPoint(h, grid.y());

        const int dx = m_widget->width() - w;
        const int dy = m_widget->height() - h;

        trySetGeometry(m_widget, m_widget->x() + dx, m_widget->y() + dy, w, h);
    } break;

    case Top: {
        if (rp.y() > pr.height() - 2 * height())
            return;

        int h = m_origGeom.height() - d.y();
        m_geom.setHeight(h);
        h = adjustPoint(h, grid.y());

        const int dy = m_widget->height() - h;
        trySetGeometry(m_widget, m_widget->x(), m_widget->y() + dy, m_widget->width(), h);
    } break;

    case RightTop: {
        if (rp.x() < 2 * width() || rp.y() > pr.height() - 2 * height())
            return;

        int h = m_origGeom.height() - d.y();
        m_geom.setHeight(h);
        h = adjustPoint(h, grid.y());

        const int dy = m_widget->height() - h;

        int w = m_origGeom.width() + d.x();
        m_geom.setWidth(w);
        w = adjustPoint(w, grid.x());

        trySetGeometry(m_widget, m_widget->x(), m_widget->y() + dy, w, h);
    } break;

    case Right: {
        if (rp.x() < 2 * width())
            return;

        int w = m_origGeom.width() + d.x();
        m_geom.setWidth(w);
        w = adjustPoint(w, grid.x());

        tryResize(m_widget, w, m_widget->height());
    } break;

    case RightBottom: {
        if (rp.x() < 2 * width() || rp.y() < 2 * height())
            return;

        int w = m_origGeom.width() + d.x();
        m_geom.setWidth(w);
        w = adjustPoint(w, grid.x());

        int h = m_origGeom.height() + d.y();
        m_geom.setHeight(h);
        h = adjustPoint(h, grid.y());

        tryResize(m_widget, w, h);
    } break;

    case Bottom: {
        if (rp.y() < 2 * height())
            return;

        int h = m_origGeom.height() + d.y();
        m_geom.setHeight(h);
        h = adjustPoint(h, grid.y());

        tryResize(m_widget, m_widget->width(), h);
    } break;

    case LeftBottom: {
        if (rp.x() > pr.width() - 2 * width() || rp.y() < 2 * height())
            return;

        int w = m_origGeom.width() - d.x();
        m_geom.setWidth(w);
        w = adjustPoint(w, grid.x());

        int h = m_origGeom.height() + d.y();
        m_geom.setHeight(h);
        h = adjustPoint(h, grid.y());

        int dx = m_widget->width() - w;

        trySetGeometry(m_widget, m_widget->x() + dx, m_widget->y(), w, h);
    } break;

    case Left: {
        if (rp.x() > pr.width() - 2 * width())
            return;

        int w = m_origGeom.width() - d.x();
        m_geom.setWidth(w);
        w = adjustPoint(w, grid.x());

        const int dx = m_widget->width() - w;

        trySetGeometry(m_widget, m_widget->x() + dx, m_widget->y(), w, m_widget->height());
    } break;

    default: break;

    } // end switch

    m_sel->updateGeometry();

    if (LayoutInfo::layoutType(m_formWindow->core(), m_widget) != LayoutInfo::NoLayout)
        m_formWindow->updateChildSelections(m_widget);
}

void WidgetHandle::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton || !m_active)
        return;

    if (m_type == TaskMenu)
        return;

    e->accept();

    if (!m_formWindow->hasFeature(FormWindow::EditFeature))
        return;

    LayoutInfo::Type layoutType = m_widget->parentWidget()
            ? LayoutInfo::layoutType(m_formWindow->core(), m_widget->parentWidget())
            : LayoutInfo::NoLayout;

    if (layoutType == LayoutInfo::Grid) {
        const QSize size = m_widget->parentWidget()->size();
        QGridLayout *grid = static_cast<QGridLayout*>(m_widget->parentWidget()->layout());

        QDesignerLayoutDecorationExtension *deco = qt_extension<QDesignerLayoutDecorationExtension*>(core()->extensionManager(), m_widget->parentWidget());

        const int index = deco->indexOf(m_widget);
        const QRect info = deco->itemInfo(index);
        const int top = deco->findItemAt(info.top() - 1, info.left());
        const int left = deco->findItemAt(info.top(), info.left() - 1);
        const int bottom = deco->findItemAt(info.bottom() + 1, info.left());
        const int right = deco->findItemAt(info.top(), info.right() + 1);

        const QPoint pt = m_origGeom.center() - m_widget->geometry().center();

        ChangeLayoutItemGeometry *cmd = 0;

        switch (m_type) {
            default: break;

            case WidgetHandle::Top: {
                if (pt.y() < 0 && info.height() > 1) {
                    cmd = new ChangeLayoutItemGeometry(m_formWindow);
                    cmd->init(m_widget, info.y() + 1, info.x(), info.height() - 1, info.width());
                } else if (pt.y() > 0 && top != -1 && grid->itemAt(top)->spacerItem()) {
                    cmd = new ChangeLayoutItemGeometry(m_formWindow);
                    cmd->init(m_widget, info.y() - 1, info.x(), info.height() + 1, info.width());
                }
            } break;

            case WidgetHandle::Left: {
                if (pt.x() < 0 && info.width() > 1) {
                    cmd = new ChangeLayoutItemGeometry(m_formWindow);
                    cmd->init(m_widget, info.y(), info.x() + 1, info.height(), info.width() - 1);
                } else if (pt.x() > 0 && left != -1 && grid->itemAt(left)->spacerItem()) {
                    cmd = new ChangeLayoutItemGeometry(m_formWindow);
                    cmd->init(m_widget, info.y(), info.x() - 1, info.height(), info.width() + 1);
                }
            } break;

            case WidgetHandle::Right: {
                if (pt.x() > 0 && info.width() > 1) {
                    cmd = new ChangeLayoutItemGeometry(m_formWindow);
                    cmd->init(m_widget, info.y(), info.x(), info.height(), info.width() - 1);
                } else if (pt.x() < 0 && right != -1 && grid->itemAt(right)->spacerItem()) {
                    cmd = new ChangeLayoutItemGeometry(m_formWindow);
                    cmd->init(m_widget, info.y(), info.x(), info.height(), info.width() + 1);
                }
            } break;

            case WidgetHandle::Bottom: {
                if (pt.y() > 0 && info.width() > 1) {
                    cmd = new ChangeLayoutItemGeometry(m_formWindow);
                    cmd->init(m_widget, info.y(), info.x(), info.height() - 1, info.width());
                } else if (pt.y() < 0 && bottom != -1 && grid->itemAt(bottom)->spacerItem()) {
                    cmd = new ChangeLayoutItemGeometry(m_formWindow);
                    cmd->init(m_widget, info.y(), info.x(), info.height() + 1, info.width());
                }
            } break;
        }

        if (cmd != 0) {
            m_formWindow->commandHistory()->push(cmd);
        } else {
            grid->invalidate();
            grid->activate();
            m_formWindow->clearSelection(false);
            m_formWindow->selectWidget(m_widget);
        }
    } else if (m_geom != m_widget->geometry()) {
        SetPropertyCommand *cmd = new SetPropertyCommand(m_formWindow);
        cmd->init(m_widget, QLatin1String("geometry"), m_widget->geometry());
        cmd->setOldValue(m_origGeom);
        m_formWindow->commandHistory()->push(cmd);
        m_formWindow->emitSelectionChanged();
    }
}

void WidgetHandle::trySetGeometry(QWidget *w, int x, int y, int width, int height)
{
    if (!m_formWindow->hasFeature(FormWindow::EditFeature))
        return;

    int minw = qMax(w->minimumSizeHint().width(), w->minimumSize().width());
    minw = qMax(minw, 2 * m_formWindow->grid().x());

    int minh = qMax(w->minimumSizeHint().height(), w->minimumSize().height());
    minh = qMax(minh, 2 * m_formWindow->grid().y());

    if (qMax(minw, width) > w->maximumWidth() ||
         qMax(minh, height) > w->maximumHeight())
        return;

    if (width < minw && x != w->x())
        x -= minw - width;

    if (height < minh && y != w->y())
        y -= minh - height;

    w->setGeometry(x, y, qMax(minw, width), qMax(minh, height));
}

void WidgetHandle::tryResize(QWidget *w, int width, int height)
{
    int minw = qMax(w->minimumSizeHint().width(), w->minimumSize().width());
    minw = qMax(minw, 16);

    int minh = qMax(w->minimumSizeHint().height(), w->minimumSize().height());
    minh = qMax(minh, 16);

    w->resize(qMax(minw, width), qMax(minh, height));
}

// ------------------------------------------------------------------------

WidgetSelection::WidgetSelection(FormWindow *parent, SelectionDictionary *selDict)   : 
    m_topWidget(0),
    m_wid(0),
    m_formWindow(parent),
    m_selectionDict(selDict),
    m_taskMenu(0)
{
    for (int i = WidgetHandle::LeftTop; i < WidgetHandle::TypeCount; ++i) {
        m_handles.insert(i, new WidgetHandle(m_formWindow, static_cast<WidgetHandle::Type>(i), this));
    }

    hide();
}

void WidgetSelection::setWidget(QWidget *w, bool updateDict)
{
    m_taskMenu = 0; // ### qt_extension<QDesignerTaskMenuExtension*>(core()->extensionManager(), w);

#ifndef NO_TOPWIDGET
    if (m_topWidget) {
        Q_ASSERT(m_topWidget->parentWidget() != 0);
        m_topWidget->parentWidget()->setAttribute(Qt::WA_ContentsPropagated, false);
    }

    delete m_topWidget;
    m_topWidget = 0;
#endif

    if (m_wid != 0)
        m_wid->removeEventFilter(this);

    if (w == 0) {
        hide();
        if (updateDict)
            m_selectionDict->remove(m_wid);
        m_wid = 0;
        return;
    }

    m_wid = w;

    m_wid->installEventFilter(this);

    const bool active = LayoutInfo::isWidgetLaidout(m_formWindow->core(), m_wid) == false;

    for (int i = WidgetHandle::LeftTop; i < WidgetHandle::TypeCount; ++i) {
        if (WidgetHandle *h = m_handles[i]) {
            h->setWidget(m_wid);
            h->setActive(active);
        }
    }

    QLayout *layout = LayoutInfo::managedLayout(m_formWindow->core(), m_formWindow->designerWidget(m_wid->parentWidget()));
    if (QGridLayout *grid = qobject_cast<QGridLayout*>(layout)) {
        const int index = grid->indexOf(m_wid);
        if (index == -1) {
#if defined(QD_DEBUG)
            qWarning() << "unexpected call to WidgetSelection::setWidget()" << "widget:" << m_wid << "grid:"<< grid;
#endif
            return;
        }

        Q_ASSERT(index != -1);

        QDesignerLayoutDecorationExtension *deco = qt_extension<QDesignerLayoutDecorationExtension*>(core()->extensionManager(), m_wid->parentWidget());
        if (deco == 0) {
            // try with the actual layout
            deco = qt_extension<QDesignerLayoutDecorationExtension*>(core()->extensionManager(), layout);
        }

        if (deco != 0) {
            // bottom cell
            m_handles[WidgetHandle::Bottom]->setActive(true);
            // top cell
            m_handles[WidgetHandle::Top]->setActive(true);
            // left cell
            m_handles[WidgetHandle::Left]->setActive(true);
            // right cellg59
            m_handles[WidgetHandle::Right]->setActive(true);
        } else {
            qWarning() << "no QDesignerLayoutDecorationExtension for widget:" << m_wid;
        }
    }

#ifndef NO_TOPWIDGET
    m_wid->setAttribute(Qt::WA_ContentsPropagated, true);
    m_topWidget = new TopWidget(wid);
    QPalette p = m_topWidget->palette();
    p.setColor(m_topWidget->backgroundRole(), QColor(255, 0, 0, 32));
    m_topWidget->setPalette(p);
#endif

    updateGeometry();
    show();

    if (updateDict)
        m_selectionDict->insert(w, this);
}

bool WidgetSelection::isUsed() const
{
    return m_wid != 0;
}

void WidgetSelection::updateGeometry()
{
    if (!m_wid || !m_wid->parentWidget())
        return;

    QPoint p = m_wid->parentWidget()->mapToGlobal(m_wid->pos());
    p = m_formWindow->mapFromGlobal(p);
    const QRect r(p, m_wid->size());

    const int w = 6;
    const int h = 6;

    for (int i = WidgetHandle::LeftTop; i < WidgetHandle::TypeCount; ++i) {
        WidgetHandle *hndl = m_handles[ i ];
        if (!hndl)
            continue;
        switch (i) {
        case WidgetHandle::LeftTop:
            hndl->move(r.x() - w / 2, r.y() - h / 2);
            break;
        case WidgetHandle::Top:
            hndl->move(r.x() + r.width() / 2 - w / 2, r.y() - h / 2);
            break;
        case WidgetHandle::RightTop:
            hndl->move(r.x() + r.width() - w / 2, r.y() - h / 2);
            break;
        case WidgetHandle::Right:
            hndl->move(r.x() + r.width() - w / 2, r.y() + r.height() / 2 - h / 2);
            break;
        case WidgetHandle::RightBottom:
            hndl->move(r.x() + r.width() - w / 2, r.y() + r.height() - h / 2);
            break;
        case WidgetHandle::Bottom:
            hndl->move(r.x() + r.width() / 2 - w / 2, r.y() + r.height() - h / 2);
            break;
        case WidgetHandle::LeftBottom:
            hndl->move(r.x() - w / 2, r.y() + r.height() - h / 2);
            break;
        case WidgetHandle::Left:
            hndl->move(r.x() - w / 2, r.y() + r.height() / 2 - h / 2);
            break;
        case WidgetHandle::TaskMenu:
            hndl->move(r.x() + r.width() - w / 2, r.y() + h - h / 2);
            break;
        default:
            break;
        }
    }

#ifndef NO_TOPWIDGET
    if (m_topWidget) {
        m_topWidget->setGeometry(wid->rect());
    }
#endif
}

void WidgetSelection::hide()
{
    for (int i = WidgetHandle::LeftTop; i < WidgetHandle::TypeCount; ++i) {
        WidgetHandle *h = m_handles[ i ];
        if (h)
            h->hide();
    }

#ifndef NO_TOPWIDGET
    if (m_topWidget)
        m_topWidget->hide();
#endif
}

void WidgetSelection::show()
{
    for (int i = WidgetHandle::LeftTop; i < WidgetHandle::TypeCount; ++i) {
        WidgetHandle *h = m_handles[ i ];
        if (h) {
            if (i == WidgetHandle::TaskMenu) {
                h->setVisible(taskMenuExtension() != 0);
                h->raise();
            } else {
                h->show();
                h->raise();
            }
        }
    }

#ifndef NO_TOPWIDGET
    if (m_topWidget) {
        m_topWidget->show();
        m_topWidget->raise();
    }
#endif
}

void WidgetSelection::update()
{
    for (int i = WidgetHandle::LeftTop; i < WidgetHandle::TypeCount; ++i) {
        WidgetHandle *h = m_handles[ i ];
        if (h)
            h->update();
    }

#ifndef NO_TOPWIDGET
    if (m_topWidget)
        m_topWidget->update();
#endif
}

QWidget *WidgetSelection::widget() const
{
    return m_wid;
}

QDesignerFormEditorInterface *WidgetSelection::core() const
{
    if (m_formWindow)
        return m_formWindow->core();

    return 0;
}

bool WidgetSelection::eventFilter(QObject *object, QEvent *event)
{
    if (object != widget())
        return false;

    switch (event->type()) {
        default: break;

        case QEvent::Move:
        case QEvent::Resize:
            updateGeometry();
            break;
    } // end switch

    return false;
}

}

#include "widgetselection.moc"

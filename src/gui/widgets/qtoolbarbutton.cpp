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

#include "qtoolbarbutton_p.h"

#include <qaction.h>
#include <qdrawutil.h>
#include <qevent.h>
#include <qmenu.h>
#include <qpainter.h>
#include <qpointer.h>
#include <qstyle.h>
#include <qstyleoption.h>

#include <private/qabstractbutton_p.h>
#define d d_func()
#define q q_func()


class QToolBarButtonPrivate : public QAbstractButtonPrivate
{
    Q_DECLARE_PUBLIC(QToolBarButton);

public:
    QPointer<QMenu> menu;
    uint usesTextLabel : 1;

    QToolBarButtonPrivate();
    QStyleOptionButton getStyleOption() const;
};

QToolBarButtonPrivate::QToolBarButtonPrivate()
    : usesTextLabel(false)
{ }

QStyleOptionButton QToolBarButtonPrivate::getStyleOption() const
{
    QStyleOptionButton opt(0);
    opt.init(q);
    if (usesTextLabel)
        opt.text = text;
    opt.icon = icon;

    if (d->down) {
        opt.state |= QStyle::Style_Down;
    } else if (d->checked) {
        opt.state |= QStyle::Style_On;
    } else if (q->isEnabled()) {
        if (q->underMouse())
            opt.state |= QStyle::Style_MouseOver;
        if (menu && menu->isVisible())
            opt.state |= QStyle::Style_Open;
    }

    if (menu)
        opt.features = QStyleOptionButton::HasMenu;

    return opt;
}


QToolBarButton::QToolBarButton(QWidget *parent)
    : QAbstractButton(*new QToolBarButtonPrivate, parent)
{
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    setFocusPolicy(Qt::NoFocus);
}

QToolBarButton::~QToolBarButton()
{ }

void QToolBarButton::setUsesTextLabel(bool enable)
{
    d->usesTextLabel = enable;
}

bool QToolBarButton::usesTextLabel() const
{
    return d->usesTextLabel;
}

void QToolBarButton::setMenu(QMenu *menu)
{
    d->menu = menu;
    if (autoMask())
        updateMask();
    update();
    updateGeometry();
}

QMenu *QToolBarButton::menu() const
{ return d->menu; }

void QToolBarButton::showMenu()
{
    update();
    d->menu->exec(mapToGlobal(q->rect().bottomLeft() + QPoint(0, QRect::rectangleMode())));
    update();
}

QSize QToolBarButton::sizeHint() const
{
    const QStyleOptionButton opt = d->getStyleOption();

    const QPixmap icon = d->icon.pixmap();
    const QString text = d->text;
    const QFontMetrics fm = fontMetrics();
    QSize sz = icon.size();
    if (d->usesTextLabel) {
        sz.rwidth() += fm.width(text);
        sz.rheight() = qMax(sz.height(), fm.lineSpacing());
    }

    return style().sizeFromContents(QStyle::CT_ToolBarButton, &opt, sz, fm, this);
}

QSize QToolBarButton::minimumSizeHint() const
{ return sizeHint(); }

bool QToolBarButton::hitButton(const QPoint &pos) const
{
    const QStyleOptionButton opt = d->getStyleOption();
    const QRect buttonRect = style().subRect(QStyle::SR_ToolBarButtonContents, &opt, this);
    return buttonRect.contains(pos);
}

void QToolBarButton::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && d->menu) {
        const QStyleOptionButton opt = d->getStyleOption();
        const QRect menuRect = style().subRect(QStyle::SR_ToolBarButtonMenu, &opt, this);
        if (menuRect.contains(event->pos())) {
            showMenu();
            return;
        }
    }
    QAbstractButton::mousePressEvent(event);
}

void QToolBarButton::mouseReleaseEvent(QMouseEvent *event)
{
    QAbstractButton::mouseReleaseEvent(event);
}

void QToolBarButton::enterEvent(QEvent *)
{
    actions()[0]->activate(QAction::Hover);
    update();
}

void QToolBarButton::leaveEvent(QEvent *)
{ update(); }

void QToolBarButton::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    const QStyleOptionButton opt = d->getStyleOption();
    style().drawControl(QStyle::CE_ToolBarButton, &opt, &p, this);
}

void QToolBarButton::actionEvent(QActionEvent *event)
{
    QAction *action = event->action();

    switch (event->type()) {
    case QEvent::ActionAdded:
        Q_ASSERT(actions().size() == 1);
        action->connect(q, SIGNAL(clicked()), SLOT(trigger()));
        // fall through intended

    case QEvent::ActionChanged:
        setText(action->text());
        setIcon(action->icon());
        setToolTip(action->toolTip());
        setStatusTip(action->statusTip());
        setWhatsThis(action->whatsThis());
        setMenu(action->menu());
        setCheckable(action->isCheckable());
        setChecked(action->isChecked());
        setEnabled(action->isEnabled());
        setShown(action->isVisible());
        setFont(actions().at(0)->font());
        break;

    default:
        break;
    }

    QAbstractButton::actionEvent(event);
}

#include "q4toolbarbutton_p.h"

#include <qaction.h>
#include <qdrawutil.h>
#include <qevent.h>
#include <qmenu.h>
#include <qpainter.h>
#include <qpointer.h>
#include <qstyle.h>

#include <private/qabstractbutton_p.h>
#define d d_func()
#define q q_func()


class Q4ToolBarButtonPrivate : public QAbstractButtonPrivate
{
    Q_DECLARE_PUBLIC(Q4ToolBarButton);

public:
    QPointer<QMenu> menu;

    Q4ToolBarButtonPrivate()
    { menu = 0; }

    // ### these should be done in the style, eventually
    void subRects(QRect &iconRect, QRect &textRect) const
    {
	const QPixmap icon = q->icon().pixmap();
	const QString text = q->text();
	const QFontMetrics fm = q->fontMetrics();

	iconRect.setSize(icon.size() + QSize(4, 4));
	iconRect.moveTopLeft(QPoint(1, 1));
	textRect.setSize(QSize(fm.width(text), fm.lineSpacing()) + QSize(4, 4));
	textRect.moveTopLeft(iconRect.topRight() + QPoint(1, 0));

	int maxh = qMax(iconRect.height(), textRect.height());
	iconRect.setHeight(maxh);
	textRect.setHeight(maxh);
    }

    void drawButton(QPainter *p)
    {
        QRect buttonRect = q->rect();
        QRect menuRect;

        if (d->menu) {
            buttonRect.setWidth(buttonRect.width() - 12);
            menuRect.setRect(buttonRect.right() + 1, buttonRect.top(), 12, buttonRect.height());
        }

	if (q->isDown()) {
	    qDrawShadePanel(p, buttonRect, q->palette(), true);
            if (d->menu) {
                qDrawShadePanel(p, menuRect, q->palette(), false);
            }
        } else if (q->underMouse() || (d->menu && d->menu->isVisible())) {
	    qDrawShadePanel(p, buttonRect, q->palette(), false);
            if (d->menu) {
                qDrawShadePanel(p, menuRect, q->palette(), d->menu->isVisible());
            }
        }

	const QPixmap icon = q->icon().pixmap();
	const QString text = q->text();
	QRect iconRect, textRect;
	d->subRects(iconRect, textRect);

	p->drawPixmap((iconRect.width() - icon.width()) / 2,
		      (iconRect.height() - icon.height()) / 2,
		      icon);
	p->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, text);
        if (d->menu) {
            q->style().drawPrimitive(QStyle::PE_ArrowDown, p, menuRect,
                                     q->palette(), QStyle::Style_Enabled);
        }
    }
};


Q4ToolBarButton::Q4ToolBarButton(QWidget *parent)
    : QAbstractButton(*new Q4ToolBarButtonPrivate, parent)
{ setFocusPolicy(NoFocus); }

Q4ToolBarButton::~Q4ToolBarButton()
{ }

void Q4ToolBarButton::setMenu(QMenu *menu)
{
    d->menu = menu;
    if (autoMask())
        updateMask();
    update();
    updateGeometry();
}

QMenu *Q4ToolBarButton::menu() const
{ return d->menu; }

void Q4ToolBarButton::showMenu()
{
    update();
    d->menu->exec(mapToGlobal(q->rect().bottomLeft() + QPoint(0, 1)));
    update();
}

QSize Q4ToolBarButton::sizeHint() const
{
    QRect iconRect, textRect;
    d->subRects(iconRect, textRect);
    return QSize(iconRect.width() + textRect.width() + (d->menu ? 12 : 0) + 2,
                 iconRect.height() + 2);
}

QSize Q4ToolBarButton::minimumSizeHint() const
{ return sizeHint(); }

bool Q4ToolBarButton::hitButton(const QPoint &pos) const
{
    QRect buttonRect = q->rect();
    QRect menuRect;

    if (d->menu) {
        buttonRect.setWidth(buttonRect.width() - 12);
        menuRect.setRect(buttonRect.right() + 1, buttonRect.top(), 12, buttonRect.height());
    }

    return buttonRect.contains(pos);
}

void Q4ToolBarButton::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == LeftButton) {
        QRect buttonRect = q->rect();
        QRect menuRect;

        if (d->menu) {
            buttonRect.setWidth(buttonRect.width() - 12);
            menuRect.setRect(buttonRect.right() + 1, buttonRect.top(), 12, buttonRect.height());
        }

        if (menuRect.contains(event->pos())) {
            showMenu();
            return;
        }
    }
    QAbstractButton::mousePressEvent(event);
}

void Q4ToolBarButton::mouseReleaseEvent(QMouseEvent *event)
{
    QAbstractButton::mouseReleaseEvent(event);
}

void Q4ToolBarButton::enterEvent(QEvent *)
{
    actions()[0]->activate(QAction::Hover);
    update();
}

void Q4ToolBarButton::leaveEvent(QEvent *)
{ update(); }

void Q4ToolBarButton::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    d->drawButton(&p);
}

void Q4ToolBarButton::actionEvent(QActionEvent *event)
{
    QAction *action = event->action();

    switch (event->type()) {
    case QEvent::ActionAdded:
        Q_ASSERT(actions().size() == 1);
        action->connect(q, SIGNAL(clicked()), SIGNAL(triggered()));
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
        break;

    case QEvent::ActionRemoved:
        hide();
        deleteLater();
        break;
    }

    QAbstractButton::actionEvent(event);
}

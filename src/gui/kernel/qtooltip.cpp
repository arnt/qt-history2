/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qtooltip.h"
#include "qlabel.h"
#include "qdesktopwidget.h"
#include "qevent.h"
#include "qhash.h"
#include "qapplication.h"
#include "qpointer.h"
#include "qtimer.h"
#include <private/qeffects_p.h>
#include "qdebug.h"

class QTipLabel : public QLabel
{
    Q_OBJECT
public:
    QTipLabel(const QString& text, QWidget* parent);
    ~QTipLabel();
    static QTipLabel *instance;

    bool eventFilter(QObject *, QEvent *);

    QBasicTimer hideTimer, deleteTimer;

    void hideTip();
protected:
    void enterEvent(QEvent*){hideTip();}
    void timerEvent(QTimerEvent *e);

};

QTipLabel *QTipLabel::instance = 0;

QTipLabel::QTipLabel(const QString& text, QWidget* parent)
    : QLabel(parent, 0, WStyle_StaysOnTop | WStyle_Customize | WStyle_NoBorder | WStyle_Tool | WX11BypassWM)
{
    delete instance;
    instance = this;
    setMargin(1);
    setAutoMask(false);
    setFrameStyle(QFrame::Plain | QFrame::Box);
    setLineWidth(1);
    setAlignment(AlignAuto | AlignTop);
    setIndent(0);
    ensurePolished();
    setText(text);
    adjustSize();
    qApp->installEventFilter(this);
    hideTimer.start(10000, this);
}

QTipLabel::~QTipLabel()
{
    instance = 0;
}

void QTipLabel::hideTip()
{
    hide();
    // timer based deletion to prevent animation
    deleteTimer.start(250, this);
}

void QTipLabel::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == hideTimer.timerId())
        hideTip();
    else if (e->timerId() == deleteTimer.timerId())
        delete this;
}

bool QTipLabel::eventFilter(QObject *, QEvent *e)
{
    switch (e->type()) {
    case QEvent::Leave:
    case QEvent::WindowActivate:
    case QEvent::WindowDeactivate:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
    case QEvent::FocusIn:
    case QEvent::FocusOut:
        hideTip();
    default:
        ;
    }
    return false;
}

void QToolTip::showText(const QPoint &pos, const QString &text, QWidget *w)
{
    if (QTipLabel::instance && QTipLabel::instance->text() == text)
        return;

    if (text.isEmpty()) {
        if (QTipLabel::instance)
            QTipLabel::instance->hideTip();
        return;
    }

    bool preventAnimation = (QTipLabel::instance != 0);
    int scr;
    if (QApplication::desktop()->isVirtualDesktop())
        scr = QApplication::desktop()->screenNumber(pos);
    else
        scr = QApplication::desktop()->screenNumber(w);

#ifdef Q_WS_MAC
    QRect screen = QApplication::desktop()->availableGeometry(scr);
#else
    QRect screen = QApplication::desktop()->screenGeometry(scr);
#endif

    QLabel *label = new QTipLabel(text, QApplication::desktop()->screen(scr));

    QPoint p = pos;
    p += QPoint(2,
#ifdef Q_WS_WIN
                24
#else
                16
#endif
        );
    if (p.x() + label->width() > screen.x() + screen.width())
        p.rx() -= 4 + label->width();
    if (p.y() + label->height() > screen.y() + screen.height())
        p.ry() -= 24 + label->height();
    if (p.y() < screen.y())
        p.setY(screen.y());
    if (p.x() + label->width() > screen.x() + screen.width())
        p.setX(screen.x() + screen.width() - label->width());
    if (p.x() < screen.x())
        p.setX(screen.x());
    if (p.y() + label->height() > screen.y() + screen.height())
        p.setY(screen.y() + screen.height() - label->height());
    label->move(p);

#ifndef QT_NO_EFFECTS
    if ( QApplication::isEffectEnabled(UI_AnimateTooltip) == false || preventAnimation)
        label->show();
    else if (QApplication::isEffectEnabled(UI_FadeTooltip)) {
        qFadeEffect(label);
    }
    else
        qScrollEffect(label);
#else
    label->show();
#endif

}

QPalette QToolTip::palette()
{
    return QApplication::palette("QTipLabel");
}

#include "qtooltip.moc"


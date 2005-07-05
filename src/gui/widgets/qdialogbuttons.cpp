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

#include "qdialogbuttons_p.h"

#include <qapplication.h>
#include <qpushbutton.h>
#include <qpointer.h>
#include <qmap.h>
#include <qvariant.h>
#include <qdialog.h>
#include <qlayout.h>
#include <qstyle.h>
#include <qstyleoption.h>

struct QDialogButtonsPrivate
{
    QMap<int, QString> text;
    QMap<QDialogButtons::Button, QWidget *> buttons;
    QPointer<QWidget> custom;
    quint32 enabled, visible;
    QDialogButtons::Button def;
    Qt::Orientation orient;
    bool questionMode;
};

QDialogButtons::QDialogButtons(QDialog *parent, bool autoConnect, quint32 buttons,
                               Qt::Orientation orient) : QWidget(parent)
{
    init(buttons, orient);
    if(parent && autoConnect) {
        QObject::connect(this, SIGNAL(acceptClicked()), parent, SLOT(accept()));
        QObject::connect(this, SIGNAL(rejectClicked()), parent, SLOT(reject()));
    }
}

QDialogButtons::QDialogButtons(QWidget *parent, quint32 buttons,
                               Qt::Orientation orient) : QWidget(parent)
{
    init(buttons, orient);
}

void
QDialogButtons::init(quint32 buttons, Qt::Orientation orient)
{
    if(buttons == All) {
        qWarning("QDialogButtons: cannot specify All by itself!");
        buttons = None;
    }
    d = new QDialogButtonsPrivate;
    d->questionMode = false;
    d->orient = orient;
    QStyleOption opt(0);
    opt.init(this);
    d->def = (Button)style()->styleHint(QStyle::SH_DialogButtons_DefaultButton, &opt, this);
    d->enabled = d->visible = buttons;
}

QDialogButtons::~QDialogButtons()
{
    delete (QWidget *)d->custom;
    delete d;
}

void
QDialogButtons::setQuestionMode(bool b)
{
    d->questionMode = b;
}

bool
QDialogButtons::questionMode() const
{
    return d->questionMode;
}

void
QDialogButtons::setButtonEnabled(Button button, bool enabled)
{
    if(enabled)
        d->enabled |= button;
    else
        d->enabled ^= button;
    if(d->buttons.contains(button))
        d->buttons[button]->setEnabled(enabled);
}

bool
QDialogButtons::isButtonEnabled(Button button) const
{
    return ((int)(d->enabled & button)) == button;
}

void
QDialogButtons::setButtonVisible(Button button, bool visible)
{
    if(visible) {
        if(d->buttons.contains(button))
            d->buttons[button]->show();
        d->visible |= button;
    } else {
        if(d->buttons.contains(button))
            d->buttons[button]->hide();
        d->visible ^= button;
    }
    layoutButtons();
}

bool
QDialogButtons::isButtonVisible(Button button) const
{
    return ((int)(d->visible & button)) == button;
}

void
QDialogButtons::addWidget(QWidget *w)
{
    QBoxLayout *lay = NULL;
    if(!d->custom) {
        d->custom = new QWidget(this);
        if(orientation() == Qt::Horizontal)
            lay = new QHBoxLayout(d->custom);
        else
            lay = new QVBoxLayout(d->custom);
        layoutButtons();
    } else {
        lay = (QBoxLayout*)d->custom->layout();
    }
    if(w->parent() != d->custom) {
        w->setParent(d->custom, 0);
        w->move(QPoint(0, 0));
        w->show();
    }
    lay->addWidget(w);
}

void
QDialogButtons::setDefaultButton(Button button)
{
    if(!((int)(d->visible & button) == button)) {
        qWarning("QDialogButtons: Button '%d' is not visible (so cannot be default)", button);
        return;
    }
    if(d->def != button) {
#ifndef QT_NO_PROPERTIES
        if(d->buttons.contains(d->def))
            d->buttons[d->def]->setProperty("default", QVariant(false));
#endif
        d->def = button;
#ifndef QT_NO_PROPERTIES
        if(d->buttons.contains(d->def))
            d->buttons[d->def]->setProperty("default", QVariant(false));
#endif
    }
}

QDialogButtons::Button
QDialogButtons::defaultButton() const
{
    return d->def;
}

void
QDialogButtons::setButtonText(Button button, const QString &str)
{
    d->text[button] = str;
#ifndef QT_NO_PROPERTIES
    if(d->buttons.contains(button))
        d->buttons[button]->setProperty("text", QVariant(str));
#endif
    layoutButtons();
}

QString
QDialogButtons::buttonText(Button b) const
{
    if(d->text.contains(b))
        return d->text[b];
    return QString(); //null if it is default..
}

void
QDialogButtons::setOrientation(Qt::Orientation orient)
{
    if(d->orient != orient) {
        d->orient = orient;
        if(d->custom && d->custom->layout())
            ((QBoxLayout*)d->custom->layout())->setDirection(orient == Qt::Horizontal ? QBoxLayout::LeftToRight :
                                                             QBoxLayout::TopToBottom);
        layoutButtons();
    }
}

Qt::Orientation
QDialogButtons::orientation() const
{
    return d->orient;
}

QWidget *
QDialogButtons::createButton(Button b)
{
    QPushButton *ret = new QPushButton(this);
    ret->setObjectName("qdialog_button");
    QObject::connect(ret, SIGNAL(clicked()), this, SLOT(handleClicked()));
    if(d->text.contains(b)) {
        ret->setText(d->text[b]);
    } else {
        switch(b) {
        case All: {
            QString txt = buttonText(defaultButton());
            if(txt.isNull()) {
                if(defaultButton() == Accept) {
                    if(questionMode())
                        txt = tr("Yes to All");
                    else
                        txt = tr("OK to All");
                } else {
                    if(questionMode())
                        txt = tr("No to All");
                    else
                        txt = tr("Cancel All");
                }
            } else {
                txt += tr(" to All"); //ick, I can't really do this!!
            }
            ret->setText(txt);
            break; }
        case Accept:
            if(questionMode())
                ret->setText(tr("Yes"));
            else
                ret->setText(tr("OK"));
            break;
        case Reject:
            if(questionMode())
                ret->setText(tr("No"));
            else
                ret->setText(tr("Cancel"));
            break;
        case Apply:
            ret->setText(tr("Apply"));
            break;
        case Ignore:
            ret->setText(tr("Ignore"));
            break;
        case Retry:
            ret->setText(tr("Retry"));
            break;
        case Abort:
            ret->setText(tr("Abort"));
            break;
        case Help:
            ret->setText(tr("Help"));
            break;
        default:
            break;
        }
    }
    return ret;
}

void
QDialogButtons::handleClicked()
{
    const QObject *s = sender();
    if(!s)
        return;

    for(QMap<QDialogButtons::Button, QWidget *>::Iterator it = d->buttons.begin(); it != d->buttons.end(); ++it) {
        if(it.value() == s) {
            emit clicked((QDialogButtons::Button)it.key());
            switch(it.key()) {
            case Retry:
                emit retryClicked();
                break;
            case Ignore:
                emit ignoreClicked();
                break;
            case Abort:
                emit abortClicked();
                break;
            case All:
                emit allClicked();
                break;
            case Accept:
                emit acceptClicked();
                break;
            case Reject:
                emit rejectClicked();
                break;
            case Apply:
                emit applyClicked();
                break;
            case Help:
                emit helpClicked();
                break;
            default:
                break;
            }
            return;
        }
    }
}

void
QDialogButtons::resizeEvent(QResizeEvent *)
{
    layoutButtons();
}

void
QDialogButtons::showEvent(QShowEvent *)
{
    layoutButtons();
}

void
QDialogButtons::changeEvent(QEvent *ev)
{
    if(ev->type() == QEvent::StyleChange)
        layoutButtons();
    QWidget::changeEvent(ev);
}

void
QDialogButtons::layoutButtons()
{
    if(!isVisible()) //nah..
        return;

    QStyle::SubElement rects[] = {
        QStyle::SE_DialogButtonAccept, QStyle::SE_DialogButtonReject,
        QStyle::SE_DialogButtonApply,  QStyle::SE_DialogButtonHelp,
        QStyle::SE_DialogButtonCustom, QStyle::SE_DialogButtonAll,
        QStyle::SE_DialogButtonRetry,  QStyle::SE_DialogButtonIgnore,
        QStyle::SE_DialogButtonAbort };
    for(unsigned int i = 0; i < (sizeof(rects) / sizeof(rects[0])); i++) {
        QWidget *w = NULL;
        if(rects[i] == QStyle::SE_DialogButtonCustom) {
            w = d->custom;
        } else {
            Button b = None;
            if(rects[i] == QStyle::SE_DialogButtonApply)
                b = Apply;
            else if(rects[i] == QStyle::SE_DialogButtonAll)
                b = All;
            else if(rects[i] == QStyle::SE_DialogButtonAccept)
                b = Accept;
            else if(rects[i] == QStyle::SE_DialogButtonReject)
                b = Reject;
            else if(rects[i] == QStyle::SE_DialogButtonHelp)
                b = Help;
            else if(rects[i] == QStyle::SE_DialogButtonRetry)
                b = Retry;
            else if(rects[i] == QStyle::SE_DialogButtonAbort)
                b = Abort;
            else if(rects[i] == QStyle::SE_DialogButtonIgnore)
                b = Ignore;
            if(b != None) {
                if(d->buttons.contains(b)) {
                    w = d->buttons[b];
                    if(!(d->visible & b)) {
                        w->hide();
                        continue;
                    }
                } else if(d->visible & b) {
                    w = createButton(b);
                    d->buttons.insert(b, w);
                } else {
                    continue;
                }
                if(w) {
                    if(b == d->def) {
                        w->setFocus();
#ifndef QT_NO_PROPERTIES
                        w->setProperty("default", QVariant(true));
#endif
                    }
                    w->setEnabled(d->enabled & b);
                }
            }
        }
        if(w) {
            w->show();
            QStyleOption opt(0);
            opt.init(this);
            w->setGeometry(style()->subElementRect(rects[i], &opt, this));
        }
    }
}

QSize
QDialogButtons::sizeHint() const
{
    ensurePolished();
    QSize s;
    if(d->custom)
        s = d->custom->sizeHint();
    QStyleOption opt(0);
    return style()->sizeFromContents(QStyle::CT_DialogButtons, &opt, s, this).
        expandedTo(QApplication::globalStrut());
}

QSize
QDialogButtons::sizeHint(QDialogButtons::Button button) const
{
    QWidget *w = NULL;
    if(d->visible & button) {
        if(!d->buttons.contains(button)) {
            QDialogButtons *that = (QDialogButtons*)this; //ick, constness..
            w = that->createButton(button);
            that->d->buttons.insert(button, w);
        } else {
            w = d->buttons[button];
        }
    }
    return w->sizeHint();
}

QSize
QDialogButtons::minimumSizeHint() const
{
    return sizeHint();
}

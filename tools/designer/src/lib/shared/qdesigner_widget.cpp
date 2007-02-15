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

#include "qdesigner_widget_p.h"
#include "formwindowbase_p.h"

#include <QtDesigner/QDesignerFormWindowInterface>
#include <QtGui/QPainter>
#include <QtGui/qevent.h>

QDesignerDialog::QDesignerDialog(QDesignerFormWindowInterface *fw, QWidget *parent) :
    QDialog(parent),
    m_formWindow(qobject_cast<qdesigner_internal::FormWindowBase*>(fw))
{
}

void QDesignerDialog::paintEvent(QPaintEvent *e)
{
    if (m_formWindow && m_formWindow->gridVisible()) {
        m_formWindow->designerGrid().paint(this, e);
    } else {
        QPainter p(this);
        p.fillRect(e->rect(), palette().brush(QPalette::Window));
    }
}

void QDesignerLabel::updateBuddy()
{
    if (myBuddy.isEmpty()) {
        QLabel::setBuddy(0);
        return;
    }

    const QList<QWidget *> widgets = qFindChildren<QWidget*>(topLevelWidget(), QString::fromUtf8(myBuddy));
    QListIterator<QWidget *> it(widgets);
    while (it.hasNext()) {
        QWidget *widget = it.next();
        if (widget && !widget->isHidden()) {
            QLabel::setBuddy(widget);
            return;
        }
    }
    QLabel::setBuddy(0);
}

QDesignerWidget::QDesignerWidget(QDesignerFormWindowInterface* formWindow, QWidget *parent)  :
    QWidget(parent),
    m_formWindow(qobject_cast<qdesigner_internal::FormWindowBase*>(formWindow))
{
    setBackgroundRole(QPalette::Window);
}

QDesignerWidget::~QDesignerWidget()
{
}

QDesignerFormWindowInterface* QDesignerWidget::formWindow() const
{
    return m_formWindow;
}

void QDesignerWidget::paintEvent(QPaintEvent *e)
{
    if (m_formWindow && m_formWindow->gridVisible())
        m_formWindow->designerGrid().paint(this, e);
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

/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "popupwidget.h"

#include <QtGui>

PopupWidget::PopupWidget(QWidget *parent)
    :QWidget(parent, Qt::Popup), childWidget(0),
    eventLoop(0), returnCode(QDialog::Rejected)
{
}

PopupWidget::~PopupWidget()
{
     if(eventLoop)
        delete eventLoop;
}

void PopupWidget::setWidget(QWidget * widget)
{
    childWidget = widget;
    QVBoxLayout *widgetLayout = new QVBoxLayout(this);
    widgetLayout->setMargin(0);
    widgetLayout->addWidget(childWidget);
}

QWidget* PopupWidget::widget() const
{
    return childWidget;
}

int PopupWidget::exec(const QRect& rect)
{
    returnCode = QDialog::Rejected;
    move(rect.bottomLeft());
    show();
    if(!eventLoop)
        eventLoop = new QEventLoop;
    eventLoop->exec();
    return returnCode;
}

void PopupWidget::closePopup()
{
    returnCode = QDialog::Accepted;
    hide();
    if(eventLoop)
        eventLoop->exit();
}

void PopupWidget::hideEvent(QHideEvent *)
{    
    if(eventLoop)
        eventLoop->exit();
}


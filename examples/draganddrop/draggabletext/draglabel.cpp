/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtGui>

#include "draglabel.h"

DragLabel::DragLabel(const QString &text, QWidget *parent)
    : QLabel(text, parent)
{
    setFrameShape(QFrame::Panel);
    setFrameShadow(QFrame::Raised);
}

void DragLabel::mousePressEvent(QMouseEvent *event)
{
    QString plainText = text(); // for quoting purposes

    QMimeData *mimeData = new QMimeData;
    mimeData->setText(plainText);

    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setHotSpot(event->pos() - rect().topLeft());

    Qt::DropAction dropAction = drag->start(Qt::CopyAction | Qt::MoveAction);
    
    if (dropAction == Qt::MoveAction) {
        close();
        update();
    }
}

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

#ifndef DRAGLABEL_H
#define DRAGLABEL_H

#include <QLabel>

class QDragEnterEvent;
class QDragMoveEvent;
class QFrame;

class DragLabel : public QLabel
{
public:
    DragLabel(const QString &text, QWidget *parent);
protected:
    void mousePressEvent(QMouseEvent *event);

private:
    QString labelText;
};

#endif

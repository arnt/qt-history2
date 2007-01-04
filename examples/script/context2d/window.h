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

#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>
class QContext2DCanvas;
class QStandardItemModel;
class QModelIndex;

class Window : public QWidget
{
    Q_OBJECT
public:
    Window(QWidget *parent = 0);
private slots:
    void selectScript(const QModelIndex &);
private:
    QContext2DCanvas *canvas;
    QStandardItemModel *scripts;
};

#endif // WINDOW_H

#ifndef LISTWIDGET_H
#define LISTWIDGET_H

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

#include <QVariant>
#include <QListWidget>
#include <QMouseEvent>
#include <QModelIndex>
#include <QString>
#include <QStringList>
#include <QWidget>
class ListWidget : public QListWidget
{
    Q_OBJECT
public:
    ListWidget(QWidget *parent = 0);
    void mousePressEvent(QMouseEvent *e);
    void swapData(const QModelIndex &from, const QModelIndex &to);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    bool isUpEnabled() const;
    bool isDownEnabled() const;
    QStringList items() const;
    void setItems(const QStringList &items);
public slots:
    void moveCurrentUp();
    void moveCurrentDown();
signals:
    void changed();
private:
    QModelIndex movedIndex;
    bool moving;
};

#endif

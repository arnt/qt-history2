/****************************************************************************
**
** Definition of simple flow layout for custom layout example.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef CARD_H
#define CARD_H

#include <qlayout.h>
#include <qlist.h>

class CardLayout : public QLayout
{
public:
    CardLayout(QWidget *parent, int dist)
	: QLayout(parent, 0, dist) {}
    CardLayout(QLayout* parent, int dist)
	: QLayout(parent, dist) {}
    CardLayout(int dist)
	: QLayout(dist) {}
    ~CardLayout();

    void addItem(QLayoutItem *item);
    QSize sizeHint() const;
    QSize minimumSize() const;
    QLayoutItem *itemAt(int) const;
    QLayoutItem *takeAt(int);
    void setGeometry(const QRect &rect);

private:
    QList<QLayoutItem*> list;
};

#endif

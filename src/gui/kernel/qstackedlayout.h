/****************************************************************************
** $Id: .emacs,v 1.3 1998/02/20 15:06:53 agulbra Exp $
**
** Definition of something or other
**
** Created : 979899
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
****************************************************************************/

#ifndef QSTACKEDLAYOUT_H
#define QSTACKEDLAYOUT_H

#include <qlayout.h>

class QStackedLayoutPrivate;

class QStackedLayout : public QLayout
{
    Q_OBJECT
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex)
    QDOC_PROPERTY(int count READ count)
public:
    QStackedLayout(QWidget *parent);
    QStackedLayout(QLayout *parentLayout);
    ~QStackedLayout();

    int addWidget(QWidget *w);

    QWidget *currentWidget() const;
    int currentIndex() const;
    int indexOf(QWidget *) const;
    QWidget *widget(int) const;
    int count() const;

    void setCurrentIndex(int);

    // abstract virtual functions:
   void addItem(QLayoutItem *item);
    QSize sizeHint() const;
    QSize minimumSize() const;
    QLayoutItem *itemAt(int) const;
    QLayoutItem *takeAt(int);
    void setGeometry(const QRect &rect);
private:
    QStackedLayoutPrivate *d;
private:
#if defined(Q_DISABLE_COPY)
    QStackedLayout(const QStackedLayout &);
    QStackedLayout &operator=(const QStackedLayout &);
#endif

};




#endif

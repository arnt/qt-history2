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

#ifndef QSTACKEDBOX_H
#define QSTACKEDBOX_H

#include <qframe.h>

class QStackedBoxPrivate;

class QStackedBox : public QFrame
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QStackedBox);
public:
    QStackedBox(QWidget *parent=0);
    ~QStackedBox();

    QWidget *currentWidget() const;
    int currentIndex() const;

    int indexOf(QWidget *) const;
    QWidget *widget(int) const;
    int count() const;

public slots:
    void setCurrentIndex(int);

protected:
    void childEvent(QChildEvent *e);

private:
#if defined(Q_DISABLE_COPY)
    QStackedBox(const QStackedBox &);
    QStackedBox &operator=(const QStackedBox &);
#endif

};


#endif

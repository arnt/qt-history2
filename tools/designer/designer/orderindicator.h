/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of Qt Designer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef ORDERINDICATOR_H
#define ORDERINDICATOR_H

#include <qwidget.h>

class FormWindow;

class OrderIndicator : public QWidget
{
    Q_OBJECT

public:
    OrderIndicator( int i, QWidget* w, FormWindow* fw );
    ~OrderIndicator();

    void setOrder( int i, QWidget* w );
    void reposition();

protected:
    void paintEvent( QPaintEvent * );
    void mousePressEvent( QMouseEvent *e );
    void updateMask();

private:
    int order;
    QWidget* widget;
    FormWindow *formWindow;
    
};

#endif

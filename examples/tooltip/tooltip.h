/****************************************************************************
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

#include <qwidget.h>
#include <qtooltip.h>


class DynamicTip : public QToolTip
{
public:
    DynamicTip( QWidget * parent );

protected:
    void maybeTip( const QPoint & );
};


class TellMe : public QWidget
{
    Q_OBJECT
public:
    TellMe( QWidget * parent = 0, const char * name = 0 );
    ~TellMe();

    QRect tip( const QPoint & );

protected:
    void paintEvent( QPaintEvent * );
    void mousePressEvent( QMouseEvent * );
    void resizeEvent( QResizeEvent * );

private:
    QRect randomRect();

    QRect r1, r2, r3;
    DynamicTip * t;
};

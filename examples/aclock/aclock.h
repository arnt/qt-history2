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

#ifndef ACLOCK_H
#define ACLOCK_H

#include <qwidget.h>
#include <qdatetime.h>

class QTimer;
class AnalogClock : public QWidget		// analog clock widget
{
    Q_OBJECT
public:
    AnalogClock( QWidget *parent=0, const char *name=0 );
    void setAutoMask(bool b);
    
protected:
    void updateMask();
    void paintEvent( QPaintEvent *);
    void mousePressEvent( QMouseEvent *);
    void mouseMoveEvent( QMouseEvent *);
    void drawClock( QPainter* );
    
private slots:
    void timeout();

public slots:
    void setTime( const QTime & t );

private:
    QPoint clickPos;
    QTime time;
    QTimer *internalTimer;
};


#endif // ACLOCK_H

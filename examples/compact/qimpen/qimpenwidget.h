/****************************************************************************
** $Id: //depot/qt/qws/util/qws/qws.h#4 $
**
** Definition of pen input method widget
**
** Created : 20000414
**
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qwidget.h>
#include <qlist.h>
#include "qimpenchar.h"

class QIMPenWidget : public QWidget
{
    Q_OBJECT
public:
    QIMPenWidget( QWidget *parent );

    void clear();
    void removeStroke();

    void addCharSet( QIMPenCharSet *cs );
    void removeCharSet( QIMPenCharSet *cs );
    void showCharacter( QIMPenChar *, int speed = 10 );
    virtual QSize sizeHint();


signals:
    void changeCharSet( QIMPenCharSet *cs );
    void beginStroke();
    void stroke( QIMPenStroke *ch );

protected slots:
    void timeout();

protected:
    enum Mode { Waiting, Input, Output };
    bool selectSet( QPoint );
    virtual void mousePressEvent( QMouseEvent *e );
    virtual void mouseReleaseEvent( QMouseEvent *e );
    virtual void mouseMoveEvent( QMouseEvent *e );
    virtual void paintEvent( QPaintEvent *e );

protected:
    Mode mode;
    bool autoHide;
    QPoint lastPoint;
    unsigned pointIndex;
    int strokeIndex;
    QTimer *timer;
    QIMPenChar *outputChar;
    QIMPenStroke *outputStroke;
    QIMPenStroke *inputStroke;
    QIMPenCharSet *currCharSet;
    QList<QIMPenStroke> strokes;
    QList<QIMPenCharSet> charSets;
};


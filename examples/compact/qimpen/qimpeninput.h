/****************************************************************************
** $Id: //depot/qt/qws/util/qws/qws.h#4 $
**
** Definition of pen input method
**
** Created : 20000414
**
** Copyright (C) 2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit Professional Edition.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#include <qframe.h>
#include <qlist.h>
#include "qimpenchar.h"

class QPushButton;
class QIMPenWidget;

class QIMPenInput : public QFrame
{
    Q_OBJECT
public:
    QIMPenInput( QWidget *parent = 0, const char *name = 0, WFlags f = 0 );
    virtual ~QIMPenInput();

    void addCharSet( const QString &fn );

    enum MultiStrokeMode { Delay = 1, Backspace = 2 };

signals:
    void key( unsigned int unicode );

    //public slots:
    //    void hideShow();

protected slots:
    void selectCharSet( QIMPenCharSet * );
    void beginStroke();
    void strokeEntered( QIMPenStroke *st );
    void processMatches();
    void setup();

protected:
    void keypress( unsigned int ch );

protected:
    bool hidden;
    QRect prefRect;
    QIMPenWidget *pw;
    QWidget *moveWidget;
    QTimer *timer;
    QPushButton *setupBtn;
    QPushButton *hideBtn;
    QList<QIMPenStroke> strokes;
    QIMPenChar *matchChar;
    QIMPenChar testChar;
    QIMPenCharSet *currCharSet;
    QList<QIMPenCharSet> charSets;
    MultiStrokeMode strokeMode;
    unsigned lastKey;
};


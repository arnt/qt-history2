/****************************************************************************
** $Id: //depot/qt/qws/util/qws/qws.h#4 $
**
** Definition of pen input method
**
** Created : 20000414
**
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qframe.h>
#include <qlist.h>
#include "qimpenchar.h"

class QPushButton;
class QIMPenWidget;
class QIMPenSetup;

class QIMPenInput : public QFrame
{
    Q_OBJECT
public:
    QIMPenInput( QWidget *parent = 0, const char *name = 0, WFlags f = 0 );
    virtual ~QIMPenInput();

    void addCharSet( const QString &fn );

    enum MultiStrokeMode { Delay = 1, Backspace = 2 };

    QSize sizeHint() const;
    
signals:
    void key( unsigned int unicode );

    //public slots:
    //    void hideShow();
protected:
    void resizeEvent( QResizeEvent * );
    
protected slots:
    void selectCharSet( QIMPenCharSet * );
    void beginStroke();
    void strokeEntered( QIMPenStroke *st );
    void processMatches();
    void setup();

protected:
    void keypress( unsigned int ch );

    //bool hidden;
    QRect prefRect;
    QIMPenWidget *pw;
    //    QWidget *moveWidget;
    QTimer *timer;
    QPushButton *setupBtn;
    QIMPenSetup *setupDlg;
    //    QPushButton *hideBtn;
    QList<QIMPenStroke> strokes;
    QIMPenChar *matchChar;
    QIMPenChar testChar;
    QIMPenCharSet *currCharSet;
    QList<QIMPenCharSet> charSets;
    MultiStrokeMode strokeMode;
    unsigned lastKey;
};


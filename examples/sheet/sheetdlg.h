/****************************************************************************
** $Id: //depot/qt/main/examples/sheet/sheetdlg.h#1 $
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef SHEETDLG_H
#define SHEETDLG_H

#include <qpushbutton.h>

class Sheet;

class MyWidget : public QWidget {
    Q_OBJECT
public:
    MyWidget( QWidget * parent = 0, const char * name = 0 );

protected:
    virtual void resizeEvent( QResizeEvent *);

private:

    Sheet *t;
    QPushButton *qb;
    void resizeHandle( QSize );

};




#endif

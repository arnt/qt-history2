/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qtoolbar.h#2 $
**
** Definition of something or other
**
** Created : 979899
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
****************************************************************************/

#ifndef QTOOLBAR_H
#define QTOOLBAR_H

#include "qwidget.h"
#include "qmainwindow.h"

class QButton;
class QBoxLayout;
class QToolBarPrivate;


class QToolBar: public QWidget
{
    Q_OBJECT
public:
    QToolBar( const char * label, 
	      QMainWindow *, QMainWindow::ToolBarDock = QMainWindow::Top,
	      bool newLine = FALSE, const char * name = 0 );
    QToolBar( QWidget * parent = 0, const char * name = 0 );
    ~QToolBar();

    void addSeparator();

    enum Orientation { Horizontal, Vertical };
    virtual void setOrientation( Orientation );
    Orientation orientation() const { return o; }

    void show();

signals:
    void useBigPixmaps( bool );
    void useTextLabels( bool );

protected:
    void paintEvent( QPaintEvent * );

private:
    void setUpGM();

    QBoxLayout * b;
    QToolBarPrivate * d;
    Orientation o;
};


#endif

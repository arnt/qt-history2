/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qtoolbar.h#9 $
**
** Definition of QToolBar class
**
** Created : 980306
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
****************************************************************************/

#ifndef QTOOLBAR_H
#define QTOOLBAR_H

#ifndef QT_H
#include "qwidget.h"
#include "qmainwindow.h"
#endif // QT_H

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
    QToolBar( const char * label, QMainWindow *, QWidget *,
	      bool newLine = FALSE, const char * name = 0, WFlags f = 0 );
    QToolBar( QMainWindow * parent = 0, const char * name = 0 );
    ~QToolBar();

    void addSeparator();

    enum Orientation { Horizontal, Vertical };
    virtual void setOrientation( Orientation );
    Orientation orientation() const { return o; }

    void show();

    QMainWindow * mainWindow();

    void setStretchableWidget( QWidget * );

protected:
    void paintEvent( QPaintEvent * );

private:
    void setUpGM();

    QBoxLayout * b;
    QToolBarPrivate * d;
    Orientation o;
    QMainWindow * mw;
    QWidget * sw;
};


#endif

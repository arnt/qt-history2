/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qtoolbar.h#24 $
**
** Definition of QToolBar class
**
** Created : 980306
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QTOOLBAR_H
#define QTOOLBAR_H

#ifndef QT_H
#include "qwidget.h"
#include "qmainwindow.h"
#endif // QT_H

class QButton;
class QBoxLayout;
class QToolBarPrivate;
class QDomElement;

class Q_EXPORT QToolBar: public QWidget
{
    Q_OBJECT
public:
    QToolBar( const QString &label,
	      QMainWindow *, QMainWindow::ToolBarDock = QMainWindow::Top,
	      bool newLine = FALSE, const char * name = 0 );
    QToolBar( const QString &label, QMainWindow *, QWidget *,
	      bool newLine = FALSE, const char * name = 0, WFlags f = 0 );
    QToolBar( QMainWindow * parent = 0, const char * name = 0 );
    ~QToolBar();

    void addSeparator();

    virtual void setOrientation( Orientation );
    Orientation orientation() const { return o; }

    void show();

    QMainWindow * mainWindow();

    virtual void setStretchableWidget( QWidget * );

    bool event( QEvent * e );
    bool eventFilter( QObject *, QEvent * );

    virtual void setLabel( const QString & );
    QString label() const;
    
    void clear();

#ifdef QT_BUILDER
    bool setConfiguration( const QDomElement& element );

    static QObject* factory( QObject* parent );
#endif // QT_BUILDER

protected:
    void paintEvent( QPaintEvent * );

private:
    virtual void setUpGM();

    QBoxLayout * b;
    QToolBarPrivate * d;
    Orientation o;
    QMainWindow * mw;
    QWidget * sw;
    QString l;

    friend QMainWindow;
};


#endif

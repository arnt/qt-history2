/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qtoolbar.h#25 $
**
** Definition of QToolBar class
**
** Created : 980306
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
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
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QTOOLBAR_H
#define QTOOLBAR_H

#ifndef QT_H
#include "qwidget.h"
#include "qmainwindow.h"
#endif // QT_H

#ifndef QT_NO_COMPLEXWIDGETS

class QButton;
class QBoxLayout;
class QToolBarPrivate;

class Q_EXPORT QToolBar: public QWidget
{
    Q_OBJECT
    Q_PROPERTY( QString label READ label WRITE setLabel )
    Q_PROPERTY( bool hStretchable READ isHorizontalStretchable WRITE setHorizontalStretchable )
    Q_PROPERTY( bool vStretchable READ isVerticalStretchable WRITE setVerticalStretchable )
    Q_PROPERTY( Orientation orientation READ orientation WRITE setOrientation )

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
    void hide();

    QMainWindow * mainWindow();

    virtual void setStretchableWidget( QWidget * );
    void setHorizontalStretchable( bool b );
    void setVerticalStretchable( bool b );
    bool isHorizontalStretchable() const;
    bool isVerticalStretchable() const;

    bool event( QEvent * e );
    bool eventFilter( QObject *, QEvent * );

    virtual void setLabel( const QString & );
    QString label() const;

    void clear();

    QSize minimumSize() const;
    QSize minimumSizeHint() const;

protected:
    void paintEvent( QPaintEvent * );
    void resizeEvent( QResizeEvent * );

signals:
    void orientationChanged( Orientation );

private slots:
    void startMoving( QToolBar *tb );
    void endMoving( QToolBar *tb );
    void popupSelected( int );
    void emulateButtonClicked();
    void updateArrowStuff();
    void setupArrowMenu();

private:
    void init();
    virtual void setUpGM();
    void paintToolBar();

    QBoxLayout * bl;
    QToolBarPrivate * d;
    Orientation o;
    QMainWindow * mw;
    QWidget * sw;
    QString l;

    friend class QMainWindow;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QToolBar( const QToolBar & );
    QToolBar& operator=( const QToolBar & );
#endif
};

#endif // QT_NO_COMPLEXWIDGETS

#endif // QTOOLBAR_H

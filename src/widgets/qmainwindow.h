/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qmainwindow.h#27 $
**
** Definition of QMainWindow class
**
** Created : 980316
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QMAINWINDOW_H
#define QMAINWINDOW_H

#ifndef QT_H
#include "qwidget.h"
#endif // QT_H

class QMenuBar;
class QToolBar;
class QStatusBar;
class QToolTipGroup;

class QMainWindowPrivate;


class Q_EXPORT QMainWindow: public QWidget
{
    Q_OBJECT
public:
    QMainWindow( QWidget * parent = 0, const char * name = 0, WFlags f = WType_TopLevel );
    ~QMainWindow();

    QMenuBar * menuBar() const;
    QStatusBar * statusBar() const;
    QToolTipGroup * toolTipGroup() const;

    virtual void setCentralWidget( QWidget * );
    QWidget * centralWidget() const;

    enum ToolBarDock { Unmanaged, TornOff, Top, Bottom, Right, Left };

    virtual void setDockEnabled( ToolBarDock dock, bool enable );
    bool isDockEnabled( ToolBarDock dock ) const;

    void addToolBar( QToolBar *, ToolBarDock = Top, bool newLine = FALSE );
    void addToolBar( QToolBar *, const QString &label,
		     ToolBarDock = Top, bool newLine = FALSE );
    void moveToolBar( QToolBar *, ToolBarDock = Top );

    void removeToolBar( QToolBar * );


    void show();

    bool rightJustification() const;
    bool usesBigPixmaps() const;

    bool eventFilter( QObject*, QEvent* );

public slots:
    virtual void setRightJustification( bool );
    virtual void setUsesBigPixmaps( bool );

    void whatsThis();

signals:
    void pixmapSizeChanged( bool );

protected slots:
    virtual void setUpLayout();

protected:
    void paintEvent( QPaintEvent * );
    void resizeEvent( QResizeEvent * );
    void childEvent( QChildEvent * );
    bool event( QEvent * );

private:
    QMainWindowPrivate * d;
    void triggerLayout();
    void moveToolBar( QToolBar *, QMouseEvent * );

    virtual void setMenuBar( QMenuBar * );
    virtual void setStatusBar( QStatusBar * );
    virtual void setToolTipGroup( QToolTipGroup * );
};


#endif

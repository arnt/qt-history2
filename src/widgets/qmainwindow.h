/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qmainwindow.h#6 $
**
** Definition of something or other
**
** Created : 979899
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
****************************************************************************/

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


class QMainWindow: public QWidget
{
    Q_OBJECT
public:
    QMainWindow( QWidget * parent = 0, const char * name = 0 );
    ~QMainWindow();

    virtual void setCentralWidget( QWidget * );
    QWidget * centralWidget() const;

    virtual void setMenuBar( QMenuBar * );
    QMenuBar * menuBar() const;

    virtual void setStatusBar( QStatusBar * );
    QStatusBar * statusBar() const;

    virtual void setToolTipGroup( QToolTipGroup * );
    QToolTipGroup * toolTipGroup() const;

    enum ToolBarDock { Top, Bottom, Right, Left };

    void setDockEnabled( ToolBarDock dock, bool enable );
    bool isDockEnabled( ToolBarDock dock ) const;

    void addToolBar( QToolBar *, const char * label,
		     ToolBarDock = Top, bool newLine = FALSE );
    void removeToolBar( QToolBar * );

    void show();

    bool usesBigPixmaps() const;

public slots:
    void setUsesBigPixmaps( bool );

protected slots:
    void setUpLayout();

protected:
    void paintEvent( QPaintEvent * );
    bool event( QEvent * );

signals:
    void internalUseBigPixmaps( bool );

private:
    QMainWindowPrivate * d;
};


#endif

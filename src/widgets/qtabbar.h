/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qtabbar.h#7 $
**
** Definition of QTabBar class
**
** Copyright (C) 1996-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QTABBAR_H
#define QTABBAR_H

#include "qwidget.h"
#include "qpainter.h"
#include "qlist.h"

struct QTabPrivate;


struct QTab
{
    virtual ~QTab();

    QString label;
    // the bounding rectangle of this - may overlap with others
    QRect r;
    bool enabled;
    int id;
};


class QTabBar: public QWidget
{
    Q_OBJECT

public:
    QTabBar( QWidget * parent = 0, const char * name = 0 );
   ~QTabBar();

    void show();

    virtual int addTab( QTab * );

    void setTabEnabled( int, bool );
    bool isTabEnabled( int ) const;

    QSize sizeHint() const;

    int currentTab() const;
    int keyboardFocusTab() const;

    QTab * tab( int );

public slots:
    void setCurrentTab( int );
    void setCurrentTab( QTab * );

signals:
    void  selected( int );

protected:
    virtual void paint( QPainter *, QTab *, bool ) const;
    virtual QTab * selectTab( const QPoint & p ) const;

    void paintEvent( QPaintEvent * );
    void mousePressEvent ( QMouseEvent * );
    void mouseReleaseEvent ( QMouseEvent * );
    void keyPressEvent( QKeyEvent * );

    QListT<QTab> * tabList();

private:
    QListT<QTab> * l;
    QTabPrivate * d;

    void paintLabel( QPainter*, const QRect&, QTab*, bool ) const;
};


#endif // QTABBAR_H

/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qtabbar.h#1 $
**
** Tab Bar, for QTabDialog and other
**
** Copyright (C) 1996 by Troll Tech AS.  All rights reserved.
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

signals:
    void selected( unsigned int );

protected:
    virtual void paint( QPainter *, QTab *, bool ) const;
    virtual QTab * selectTab( const QPoint & p ) const;

    void paintEvent( QPaintEvent * );
    void mousePressEvent ( QMouseEvent * );
    void mouseReleaseEvent ( QMouseEvent * );

private:
    QListT<QTab> * l;
    QTabPrivate * d;
};

#endif

/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qtabbar.h#33 $
**
** Definition of QTabBar class
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

#ifndef QTABBAR_H
#define QTABBAR_H

#ifndef QT_H
#include "qwidget.h"
#include "qpainter.h"
#include "qlist.h"
#include "qiconset.h"
#endif // QT_H

struct QTabPrivate;


class Q_EXPORT QTab
{
public:
    QTab():  enabled( TRUE ), id( 0 ), iconset(0) {}
    virtual ~QTab();

    QString label;
    // the bounding rectangle of this - may overlap with others
    QRect r;
    bool enabled;
    int id;

    // an optional iconset
    QIconSet* iconset;
};


class Q_EXPORT QTabBar: public QWidget
{
    Q_OBJECT

public:
    QTabBar( QWidget * parent = 0, const char *name = 0 );
   ~QTabBar();
    
    enum Shape { RoundedAbove, RoundedBelow,
		 TriangularAbove, TriangularBelow };

    Shape shape() const;
    virtual void setShape( Shape );
    
    void show();

    virtual int addTab( QTab * );
    virtual int insertTab( QTab *, int index = -1 );
    virtual void removeTab( QTab * );

    virtual void setTabEnabled( int, bool );
    bool isTabEnabled( int ) const;

    QSize sizeHint() const;
    QSizePolicy sizePolicy() const;

    int currentTab() const;
    int keyboardFocusTab() const;

    QTab * tab( int );

    virtual void layoutTabs();

public slots:
    virtual void setCurrentTab( int );
    virtual void setCurrentTab( QTab * );

signals:
    void  selected( int );

protected:
    virtual void paint( QPainter *, QTab *, bool ) const; // ### not const
    virtual void paintLabel( QPainter*, const QRect&, QTab*, bool ) const;

    void focusInEvent( QFocusEvent *e );
    void focusOutEvent( QFocusEvent *e );

    virtual QTab * selectTab( const QPoint & p ) const;
    void updateMask();

    void resizeEvent( QResizeEvent * );
    void paintEvent( QPaintEvent * );
    void mousePressEvent ( QMouseEvent * );
    void mouseReleaseEvent ( QMouseEvent * );
    void keyPressEvent( QKeyEvent * );

    QList<QTab> * tabList();

private slots:
    void scrollTabs();

private:
    QList<QTab> * l;
    QList<QTab> * lstatic;
    void makeVisible( QTab* t  );
    void updateArrowButtons();
    QTabPrivate * d;

};


#endif // QTABBAR_H

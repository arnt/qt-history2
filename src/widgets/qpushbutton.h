/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qpushbutton.h#5 $
**
** Definition of QPushButton class
**
** Author  : Haavard Nord
** Created : 940221
**
** Copyright (C) 1994 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QPUSHBT_H
#define QPUSHBT_H

#include "qbutton.h"


class QPushButton : public QButton
{
    Q_OBJECT
public:
    QPushButton( QView *parent=0, const char *name=0 );
    QPushButton( const char *label, QView *parent=0, const char *name=0 );

    void    setAutoDefault( bool autoDef );
    bool    isAutoDefault() const { return autoDefButton; }

    void    setDefault( bool def );
    bool    isDefault() const { return defButton; }

    void    resizeFitLabel();

  // Reimplemented move,resize etc. because of auto-sized default push buttons
    bool    move( int, int );
    bool    move( const QPoint & );
    bool    resize( int, int );
    bool    resize( const QSize & );
    bool    changeGeometry( int, int, int, int );
    bool    changeGeometry( const QRect & );

signals:
    void    becameDefault();

protected:
    void    drawButton( QPainter * );
    virtual void drawButtonFace( QPainter * );

private:
    void    init();

    uint    autoDefButton : 1;
    uint    defButton	  : 1;
    uint    lastDown	  : 1;
    uint    lastDef	  : 1;
};


inline bool QPushButton::move( const QPoint &p )
{
    return move( p.x(), p.y() );
}

inline bool QPushButton::resize( const QSize &s )
{
    return resize( s.width(), s.height());
}

inline bool QPushButton::changeGeometry( const QRect &r )
{
    return changeGeometry( r.left(), r.top(), r.width(), r.height() );
}


#endif // QPUSHBT_H

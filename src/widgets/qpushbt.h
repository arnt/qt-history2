/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qpushbt.h#1 $
**
** Definition of QPushButton class
**
** Author  : Haavard Nord
** Created : 940221
**
** Copyright (C) 1994 by Troll Tech as.  All rights reserved.
**
*****************************************************************************/

#ifndef QPBUTTON_H
#define QPBUTTON_H

#include "qbutton.h"


class QPushButton : public QButton
{
    Q_OBJECT
public:
    QPushButton( QView *parent=0, const char *label = 0 );
    QPushButton( QView *parent, const QRect &r, const char *label = 0 );

    void    setAutoDefault( bool autoDef );
    bool    isAutoDefault() const { return autoDefButton; }

    void    setDefault( bool def );
    bool    isDefault() const { return defButton; }

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
    bool    extraSize( int &, int &, bool );
    void    fixDefButton();

    int	    autoDefButton : 1;
    int	    defButton	  : 1;
    int	    lastDown	  : 1;
    int	    lastDef	  : 1;
};


inline bool QPushButton::move( const QPoint &p )
{
    return move( p.getX(), p.getY() );
}

inline bool QPushButton::resize( const QSize &s )
{
    return resize( s.getWidth(), s.getHeight());
}

inline bool QPushButton::changeGeometry( const QRect &r )
{
    return changeGeometry( r.left(), r.top(), r.width(), r.height() );
}


#endif // QPBUTTON_H
